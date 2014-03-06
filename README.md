#Software Systems Homework 3
####Noam Rubin and Cory Dolphin

This is a simple C interface to a local instance of the [neo4j](http://www.neo4j.org) graph database.

###neo4j

To get neo4j, follow [these instructions](http://www.neo4j.org/download/linux)

Then, point your browser at `localhost:7474` and start the Movie Graph tutorial

Once you've added some data, try compiling and running query.c

##query.c

All the magic is in `query.c`, which depends on `curllib` for HTTP requests and `jansson` for JSON parsing. The `Makefile` links to the correct libraries, but to make sure they're installed on your system:

1. Install `curllib`
  ```
  sudo apt-get install libcurl4-gnutls-dev
  ```

2. Download and install [Jansson](https://jansson.readthedocs.org/en/2.5/gettingstarted.html#compiling-and-installing-jansson)
  - download the latest tarball: http://www.digip.org/jansson/releases/jansson-2.6.tar.gz
  - unpack the ball and cd into the directory: 
  ``` 
  bunzip2 -c jansson-2.5.tar.bz2 | tar xf
  cd jansson-2.5
  ```
  - Then install the package (you may need to be root for some of these steps):
  ```
  ./configure
  make
  make check
  make install
  ```

3. Install [Neo4j](http://www.neo4j.org/download)

4. Insert the data:
```bash
$ python parse.py
$ python insert.py
```

5. Run `make` in this directory.
6. Run the executable `query` and input a starting node name. 