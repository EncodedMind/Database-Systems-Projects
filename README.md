# Database Systems Projects

This repository contains three separate assignment projects that are kept in their original folder structure:

- `Heap/` for the heap-file assignment
- `B+ trees/` for the B+ tree assignment
- `Chunk/` for the external sorting assignment

Each folder has its own README and Makefile with the detailed implementation notes, build commands, and assumptions for that specific task.

The three projects are related, but they are not identical. They share the same general database-systems theme and some common BF / record concepts, yet each assignment uses its own API layer and source files. Keeping them separate avoids conflicts where files with the same name have different contents.

Use the root `Makefile` if you want a single entry point to build or run one of the three parts.