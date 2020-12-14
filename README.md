# HRML parser

## Description

This sample application parses text in hrml format.
The description of the task was suggested by
[the link](https://www.hackerrank.com/challenges/attribute-parser/problem).
Application can use two approaches: The first with tag tree building,
the second with direct stream search. Approaches are switched with
`--parser` option.

## Building application

To build application, CMake minimum version 3.10 and compiler with  C++17 support required.

- ```git clone https://github.com/idales/hrml_parser```to clone repository.
- ```mkdir build && cd build``` in repository folder to create build folder.
- ```cmake ..``` to create cmake configuration.
- ```cmake --build .``` to build the application.

## Command line parameters

Usage: `hrml_parser [--parser|-p <parser type>] <input filename>`

`<parser type>` could be `"tag_tree"` or `"in_stream"`

- various algorithms for finding attribute value
  
`<input filename>` input file with following format

- Lines in HRML format
- Empty line
- Lines with queries in the format `<tag>[.<tag>]... ~<attribute>`
- Empty line

## HRML format

HRML is artifical markup language. Each line has only one opening or closing tag.
 Opening tags can also contain attributes in form `<attribute name> = "<attribute value>"`.
 Tags can be nested.

## Sample input and output

Input file can be as follows:

            <tag1 value = "Hello World">
            <tag2 name = "Name1">
            </tag2>
            <tag3 another="another" final="final">
            </tag3>
            </tag1>

            tag1.tag2~name
            tag1~name
            tag1~value

Output should be:

            tag1.tag2~name="Name1"
            tag1~name not found
            tag1~value="Hello World"
