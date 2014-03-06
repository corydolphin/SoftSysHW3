# -*- coding:utf-8 -*-

import requests
from requests import ConnectionError
import json
import csv
import time
from multiprocessing.pool import ThreadPool
from multiprocessing.pool import Pool

def post(url, data={}, num_retries=10):    
    '''
        A POST wrapper for the requests library which transparently retries in
        the case of failure.
    '''
    for i in range(num_retries):
        try:
            return requests.post(url,data=json.dumps(data), headers={
                'Content-Type': 'application/json',
                'Accept' : 'application/json; charset=UTF-8'
                })
        except ConnectionError:
            print "!!!!Failed request, retrying %s times" % (num_retries-1 -i)
            time.sleep(2*i)
    print "!!!!!!!Failed 10 times"
    return None

def create_movie(movie, date):
    '''
    Creates a movie node if unique, or returns the node with the same title
    '''
    r = post('http://localhost:7474/db/data/index/node/Movie?uniqueness=get_or_create',
        data={
            "key" : "name",
            "value" : movie,
            "properties" : {
                "name" : movie,
                "date" : date
            }})
    if r.status_code == 500:
        print "500 error on create_movie"
    res = r.json()
    labels_url = res[u'labels']
    r = post(labels_url, data=["Movie"])
    return res['self']

def create_actor(actor):
    '''
    Creates an actor node if unique, or returns the node with the same name
    '''
    r = post('http://localhost:7474/db/data/index/node/Actor?uniqueness=get_or_create',
        data={
            "key" : "name",
            "value" : actor,
            "properties" : {
                "name" : actor,
            }
        })
    if r.status_code == 500:
        print "500 error on create_actor"
    res = r.json()
    labels_url = res[u'labels']
    r = post(labels_url, data=["Person"])
    return res['self']


def create_relationship(actor_url,movie_url,role):
    '''
    Creates a relationship between actor_url and movie_url with an attribute of 'role'
    '''
    r = post(actor_url + '/relationships',{
        "to" : movie_url,
        "type" : "ACTED_IN",
        "data" : {
            "roles" : [role]
        }
        })
    if r.status_code == 500:
        print "500 error on create_relationship"
    res = r.json()


def processLines(itrarg):
    '''
        Processes a group of lines
    '''
    map(processLine, itrarg)

def processLine(arg):
    '''
        Processes a single line, creating or getting an actor, movie and
        creating the relationship between the two.
    '''
    i, (actor,title,date,role) = arg
    if i % 10000 == 0:
        print i

    if not title or not date or not role or not actor:
        return
    try:
        actor = actor.decode('utf-8')
        actor_url = create_actor(actor)
        movie_url = create_movie(title, date)
        create_relationship(actor_url, movie_url,role)
    except UnicodeDecodeError:
        pass    


def iterlines():
    '''
        An iterator over the combined, parsed file.
    '''
    with open('combined.txt', 'r') as infile:
        csvfile = csv.reader(infile)
        for i, (actor,title,date,role) in enumerate(csvfile):
            yield actor,title,date,role
            if i % 10000 == 0:
                print "Completed %s inserts" % i

def grouper(n, iterable):
    '''
        Splits an iterable into groups of size n.

        params: n, number per group
        params: iterable, an iterable to chunk

        returns an iterator of iterators of length n until the iterable
        is exhausted.
    '''
    iterable=iter(iterable)
    while True:
        result=[]
        for i in range(n):
            try:
                a=next(iterable)
            except StopIteration:
                break
            else:
                result.append(a)
        if result:
            yield result
        else:
            break

def main():
    pool = Pool(processes=4)
    with open('combined.txt', 'r') as infile:
        csvfile = csv.reader(infile)
        pool.map(processLines, grouper(5000, enumerate(csvfile)))


if __name__ =='__main__':
  main()