# Plinth - Base layer library

Plinth is a base layer library for small to midsize C programming
projects. Most of the Plinth features are for memory allocations, but
it also provides facilities for string handling and basic type
definitions.

Plinth allows the memory allocators to be nested, i.e. arrangements
where the nested allocator allocates from the base allocator. Nesting
provides flexible and efficient memory allocations.

Allocators are used through handles, which are pointers to Allocator
Structs. Allocators return persistent references, except with the
Continuous Memory Allocator (details below).

See Doxygen docs and `plinth.h` for details about Plinth API. Also
consult the test directory for usage examples.


## Basic Memory Allocation

Plinth defines basic heap memory allocation functions:
`pl_alloc_memory`, `pl_free_memory`, and `pl_realloc_memory`. These
are equal to `calloc`, `free,` and `realloc`. Additionally there is
`pl_strdup` for string duplication and `pl_format` for formatting
strings to heap allocations. Both `pl_strdup` and `pl_format` results
must be deallocated with `pl_free_memory`.


## Arena Memory Allocator

`plam` is the fundamental memory allocator in Plinth. It is an Arena
type of allocator. `plam` consists of one or more Nodes, which are
chained as doubly linked list. Each allocation from `plam` is a
continous chunk of memory from a Node. `plam` can be created from heap
with `plam_new` and from a pre-existing allocation with `plam_use`
(nesting). Heap allocated `plam` has Debt, i.e. it needs to be
deallocated from heap. If `plam` uses pre-existing allocation (for
example, from stack or from another `plam`), it does not require a
deallocation.

```
           plam_node_s
          / used mem
         / /         ,unused mem
        #+++- <-> #+---
        '---'     '---'
          \         \
           node      current node
           size
```

The allocation is performed with `plam_get`. When the current Node run
out of memory, a new Node is allocated. User can also put back
allocations with `plam_put`. The deallocations must occur in reverse
order to the allocations and they have to be annotated with correct
allocation sizes.

The complete chain of Nodes is deallocated with `plam_del`. `plam_del`
does real deallocation only when the `plam` Node has Debt. A common
pattern is where the first Node has no Debt, but all the rest have
Debt.


## Block Memory Allocator

`plbm` is a block memory allocator. It allocates "small" blocks of
memory with fixed sizes. The blocks can be allocated and deallocated
in any order. Under the hood, the deallocated blocks are maintained in
a linked list. Each block is a continuous chunk of memory, but memory
between blocks is not guaranteed to be continuous. However,
back-to-back allocations (no deallocations in between) from a
particular Node are continuous.

```
           ..       .-.
           v|       | v
        #++-- <-> #+-+-
           |^       ^ |
           o'-------|-'
                    head
```

`plbm` can be created to heap with `plbm_new` (Debt) or to
pre-allocated memory with `plbm_use` (no Debt). `plbm` shares the same
Node structure as `plam`.

A block of memory is allocated with `plbm_get` and deallocated with
`plbm_put`. Every deallocated block becomes part of the
free-block-chain, which is used as priority for every new block
allocation. When there are no blocks in the chain or available blocks
in the current Node, a new Node is allocated and a block is returned
from the new Node.


## Continous Memory Allocator

`plcm` is a Continuous memory allocator. `plcm` can be created to heap
with `plcm_new` (Debt) or to pre-allocated memory with `plcm_use` (no
Debt). `plcm` guarantees that all the allocated memory is continuous.
`plcm` is therefore suitable for strings and other arrays with packed
items.

```
           plcm_s    data
          /         /    used
         /         /    /   ,size
        #    =>   +++++-----
```

`plcm` is created with initial allocation size. The user makes
allocations from the initial allocation, until it runs out. Regardless
of the initial allocation storage (Debt or not), the next allocation
is always from heap (with Debt). When the allocation is renewed, it
can change the location of the allocation, and therefore all direct
references to memory become invalid. For this reason, references to
allocations are by default offsets to base address. The offsets are
called Positions. When Position is used, it is added to the current
base address and resulting references are valid.

Allocations can be made with `plcm_get_pos`, where the return value is
a Position to the allocator. Other possibility is `plcm_get_ref`,
where the return value is a pointer to the allocated location. User
may also allocate and store the value in one action with
`plcm_store` function.

If the continuous data storage requires a terminating value,
`plcm_terminate` can be used. For example, a NULL terminated array
could be achieved with `plcm_terminate`.


## String Storage

`plss` provides a set of functions for storing string data to a
`plcm`. User can append to existing strings (in `plcm`) with
`plss_append`, `plss_append_c`, `plss_append_ch`, or with
`plss_format`.

The existing string can be replaced with `plss_set` or
`plss_reformat`.

The stored string content is retreived with `plss_string` and length
with `plss_length`.

Each `plss` storage function ensures that the stored string is NULL
terminated and can be used directly with C library functions.


## String References

`plsr` provides a set of functions for operating with immutable string
references. A string reference is a pair of string content and string
length. String references may point to any location in a C-string,
since it contains the explicit length and does not require the NULL
terminator.

String reference provides an efficient way of examining and
constructing strings. String references are used by value, which
allows function call nesting and in overall, more functional style
programming.


## Plinth API documentation

See Doxygen documentation. Documentation can be created with:

    shell> doxygen .doxygen


## Examples

All functions and their use is visible in tests. Please refer `test`
directory for testcases.


## Building

Ceedling based flow is in use:

    shell> ceedling

Testing:

    shell> ceedling test:all

User defines can be placed into `project.yml`. Please refer to
Ceedling documentation for details.


## Ceedling

Plinth uses Ceedling for building and testing. Standard Ceedling files
are not in GIT. These can be added by executing:

    shell> ceedling new plinth

in the directory above Plinth. Ceedling prompts for file
overwrites. You should answer NO in order to use the customized files.
