# Automatic Index Creation

## Team Members
- Saksham Rathi (22B1003)
- Kavya Gupta (22B1053)
- Shravan S (22B1054)
- Mayank Kumar (22B0933)

## Directory Structure
Here is the directory structure of the submission:

- `./code`: Contains the header and C++ files for the implementation, along with the Makefile. It also contains some python files for parsing, and running queries.
- `./theory`: Contains some relevant papers and slides.  
- `./documentation`: Contains the report as `readme.pdf`.  
- `./README.md`: Contains the instructions to run the code.

## How to setup?
- Install psql on your machine through this [link](https://www.postgresql.org/download/linux/ubuntu/).
- Install hypopg extension (for calculating query costs) from this [link](https://hypopg.readthedocs.io/en/rel1_stable/installation.html).
- Install the following python libraries:
```
psycopg2
sqlparse
```

## How to run the code
```
cd code
make clean
make
./run
```

This will start our shell, which can accept queries, and will execute them based on the indices created. You can use up arrow, if you want to execute one of the previous queries again.
