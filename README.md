# JSON to Relational CSV Converter

## Overview

This tool converts any valid **JSON file** into **relational CSV files**, following clear normalization rules. It uses **Flex** for lexical analysis, **Yacc/Bison** for parsing, and **C** for AST construction and CSV generation.

---

## Features

* Parses **valid JSON up to 30 MiB**
* Builds a persistent **Abstract Syntax Tree (AST)**
* Follows defined **conversion rules** for normalization
* Streams **CSV output** directly without large memory buffers
* Tracks **line and column** for error messages
* Optional flag to **print AST**
* Generates one `.csv` file per relational table
* Memory-safe C code

---

## Conversion Rules

| JSON Structure   | Relational Mapping                        |
| ---------------- | ----------------------------------------- |
| Object           | Table row                                 |
| Array of objects | Child table with foreign key              |
| Array of scalars | Junction table (parent\_id, index, value) |
| Scalars          | Column values (null → empty)              |
| Row Identifiers  | `id` as primary key                       |
| Foreign Keys     | `<parent>_id` in child table              |

---

## Pre requisites
```sh
brew install flex bison
```

---

## Compilation Instructions

Make sure you have `flex`, `bison`, and `gcc` installed.

```bash
bison -d parser.y
flex scanner.l
gcc -o json2relcsv main.c parser.tab.c lex.yy.c ast.c symbol_table.c -ly -ll
```

---

## Usage

```bash
./json2relcsv --print-ast --print-symbol-table --out-dir DIR < input.json 
```

### Options

* `--print-ast`: Prints the abstract syntax tree to stdout.
* `--print-symbol-table`: Prints the symbol table.
* `--out-dir DIR`: Sets the output directory for CSV files (default: current directory).

---

## Examples

### Input

```json
{
  "orderId": 7,
  "items": [
    {"sku": "X1", "qty": 2},
    {"sku": "Y9", "qty": 1}
  ]
}
```

### Output

`orders.csv`:

```
id,orderId
1,7
```

`items.csv`:

```
order_id,seq,sku,qty
1,0,X1,2
1,1,Y9,1
```

---

## Error Handling

* Reports **first JSON error** with **line and column**
* Exits with **non-zero status** on invalid input

---



