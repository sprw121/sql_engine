This project is a command line sql engine
for csv files. Supports loading of csvs with
integer or floating point columns. Column
headers for required. Currently implements
basic SELECT queries on them for maniuplation.
Can be run in an interactive terminal mode,
or a cli only mode.

The program takes arguments of the form
table1=csv1 table2=csv2 ...
and pre-loads those csvs as tables that
we can execute queries on. The cli only
mode can be invoked by following the
table argument list with:

--execute "query;"

This will cause the results of query to
be written to stdout as a csv. The use
case here would likely be piping or redirecting
the results of the query into a new file.

The command for such a use case would look like.
./csv_sql trades=trades.csv --execute "select * from trades;"

Omitted the --execute will execute an interactive
terminal, with the table arguments already loaded.

Commands must be terminated with a semi-colon.

A simple compile script is included. This project
was built and tested with:
g++ (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609
-std=c++11

Some of the ways that the data is stored and accessed
to support multiple column types may vary from compiler
to compiler or platform to platform.

Currently supported query commands are:
SELECT, LOAD, DESCRIBE, SHOW, EXIT.


SELECT
------------
SELECT takes any number of column expressions
to generate output, a FROM clause, and optional
WHERE, OFFSET and LIMIT clauses.

Column expressions maybe arithmetic expressions
of columns from the FROM clause, or an aggregate
of and arithmetic expression. Expressions of aggregates
are not supported. Currently implemented aggregates are
max, min, average, median. Column expressions maybe named
with an as clause, which caused them to be named
that in the output. Otherwise they are named col_n, where
n is there column index.

FROM clause can take in a table, select, join. Joins maybe
non-trivial, so we can join on joins, or select statements.
Currently implemented joins are INNER_JOIN, OUTER_JOIN,
LEFT_JOIN, RIGHT_JOIN, CROSS_JOIN. The former 4 must
have an ON clause of the form left.column = right.column,
as the index to join on. Only joins with integer column keys
are implemented.

WHERE clause takes in an arbitrary number of boolean
expressions to describe filtering of the SELECT.
Boolean expressions should be on columns referencing the FROM
clause.

LIMIT and OFFSET each take in integer values. LIMIT N
limits the output of the select to N columns. OFFSET M
requires a LIMIT clause, and begins the output at row M.

Queries such as the following are all supported:

SELECT * from trades;
SELECT TIME, PRICE, QUANTITY from trades;
SELECT TIME, ASK - BID from quotes;
SELECT max(PRICE), min(ASK), median(TIME) from trades join quotes on TIME = TIME;
SELECT * from trades join quotes on TIME = TIME join quotes on trades.TIME = TIME LIMIT 10;
SELECT * from (SELECT * from trades LIMIT 10 OFFSET 10) LIMIT 5;
SELECT ASK * BID + TIME from quotes;
SELECT TIME as t, BID as b, ASK as a from quotes;
SELECT * from trades where TIME > 5, PRICE > 10.01;

We can do pretty arbitrary things based on the above rules;

LOAD
----------
LOAD takes in an arbitary number of AS clauses and loads
tables. The AS clauses should be of the form csv AS table.

The command:

LOAD trades.csv as trades, quotes.csv as quotes;

would load file trades.csv as table trades, and quotes.csv
as table quotes.


DESCRIBE
---------
DESCRIBE takes in any of tables names, and outputs the column
names and types in those table.

The command:

DESCRIBE tables1, tables2;

would describe both tables in order.

SHOW
------
The only valid SHOW command is

SHOW TABLES;

This outputs the currently loaded tables.

EXIT
------
The only valid EXIT command is

EXIT;

This exits the program.

