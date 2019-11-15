# prototype-db-1

This is my prototype database #1.
It will be hardly a database but it will help to get familiar to how to structure the database implementation.

## Goals

- Building a working simple in-memory key-value store

## Design Choices

- Data Model: a collection of pairs of a binary key and a binary value
- Operations: get/set/delete
- Main Table: hashtable, single-version
- Protocol: Redis protocol (GET/SET/DEL)
- Supports concurrent access: multi-threaded
- No index support.
