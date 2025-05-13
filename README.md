# Plinth - Base layer library

Plinth is a base layer library for small to midsize C programming
projects. Most of the Plinth features are for memory allocations and
storage, but it also provides facilities for string handling and basic
type definitions.

Plinth allows the memory allocators to be nested, i.e. arrangements
where the nested allocator allocates from the host (base) allocator.
Nesting provides flexible and efficient memory allocations. In
general, pre-existing allocations can be used for allocators.
Pre-existing allocation can be from stack or from another allocator.

Allocators are used through handles, which are pointers to Allocator
Descriptors. All allocators return persistent references, except
the Continuous Memory Allocator (details below).

See Doxygen docs and `plinth.h` for details about Plinth API. Also
consult the test directory for usage examples.


## Basic Memory Allocation

Plinth defines basic heap memory allocation functions:
`pl_alloc_memory`, `pl_free_memory`, and `pl_realloc_memory`. These
are equal to `calloc`, `free,` and `realloc`. Additionally there is
`pl_alloc_string` for string duplication and `pl_format_string` for
formatting strings to heap allocations. Both `pl_alloc_string` and
`pl_format_string` results must be deallocated with `pl_free_memory`.


## Arena Memory Allocator

`plam` is the fundamental memory allocator in Plinth. It is an Arena
type of allocator. `plam` consists of one or more Nodes, which are
chained together as a doubly linked list. Each allocation from `plam`
is a continuous chunk of memory from a Node. `plam` can be created
from heap with `plam_new` (Dept) or from a pre-existing allocation
with `plam_use` (no Dept). Heap allocated `plam` has Debt, i.e. it
needs to be deallocated from heap. If `plam` uses pre-existing
allocation (for example, from stack or from another `plam`), it does
not require a deallocation.

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

The allocation is performed with `plam_get`. When the current Node
runs out of memory, a new Node is allocated (from heap). User can also
put back allocations with `plam_put`. The deallocations must occur in
reverse order to the allocations and they have to be annotated with
the corresponding allocation sizes.

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
from the new Node. The blocks are returned in order, and therefore it
is possible for user to get continuous memory.


## Continuous Memory Allocator

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
base address and therefore the resulting references are up-to-date and
valid.

Allocations can be made with `plcm_get_pos`, where the return value is
a Position to the allocator. Other possibility is `plcm_get_ref`,
where the return value is a pointer to the allocated location. User
may also allocate and store the value in one action with
`plcm_store` function.

If the continuous data storage requires a terminating value,
`plcm_terminate` can be used. For example, a NULL terminated array
could be achieved with `plcm_terminate`.

`plcm` may contain multiple allocations, but typically there is only
one allocation, which changes its size. The only allocation that can
change its size in `plcm` is the last. Only allocation is also the
last.


## String Storage

`plss` provides a set of functions for storing string data to a
`plcm`. User can append to existing strings (in `plcm`) with
`plss_append`, `plss_append_string`, `plss_append_char`, or with
`plss_format_string`.

The existing string can be replaced with `plss_set` or
`plss_reformat_string`.

The stored string content is retrieved with `plss_string` and length
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
constructing strings. String references are used by value, i.e. the
`plsr` struct is copied from caller to callee. This allows function
call nesting and in overall, more functional style of programming.


## Memory Allocation Strategies

Program performance is often limited by memory bandwidth and latency.
We want to get the data from the nearest possible cache level. This is
enabled by minimizing the amount of data in use, promoting data
locality and laying out the data to consecutive addresses (arrays).

Each allocated data has size and lifetime. The easiest case is when we
know the maximum data size and the exact lifetime, in advance. The
most difficult case is when we don't know the data size and the
lifetime might vary.

Strings, and other array types, needs to be allocated to a continuous
chunk of memory. This is possible with `plam`, when we know the size
in advance. For unknown sizes, we should use the `plcm`.

Independent objects can be allocated from `plbm`. `plbm` allows also
efficient and convenient allocation/deallocation sequences, since all
allocations can be deallocated in any order.

Stack allocations should be used much as possible. However, stack
allocations should be reserved for small data that is discarded at
function exit. Recursive functions should not normally make much stack
allocations. Array of memory from stack can be especially useful for
the `plcm` allocator. For example, if a string is build within the
function, the user can allocate from stack an char array. The size is
adjusted so that the typical string fits into the array. In the rare
occasion when the stack memory is not enough, the allocation is moved
to heap automatically by `plcm`. The typical case remains fast, since
no heap allocation is made.

When the data lifetime exceeds the lifetime of the allocating
function, the data must be allocated from heap. Also when the size of
the data is large, we should not allocate from stack and use heap
instead. Direct heap allocations with `pl_alloc_memory` (i.e.
`malloc`), should be avoided when possible. A common strategy is to
first allocated large amount of with `plam` (the host) and then create
nested allocators to the host. If the allocated objects are
independent, we can use a `plbm`. If the we need an array, we can use
a `plcm`. The initial size of `plcm` should be large enough to cover
the typical use cases. However, if `plcm` capacity is exhausted, the
allocation is moved to heap. In this case the initial `plcm`
allocation becomes overhead in the host `plam`.

When data lifetime is equal to the program lifetime, the allocation
strategy becomes simpler. It really doesn't matter how memory is
allocated.

Temporary string building buffers can be created with `plcm`. The
initial reservation should be sufficiently large. Fixed sized
allocation, which also needs to be deallocated, can be created with a
`plbm`. Reoccurring allocations can be placed to a `plam` with
sufficiently large initial size, from which numerous smaller
allocations can be made from. This helps in avoiding to perform the
more expensive direct heap allocations (`pl_alloc_memory`). When data
lifetime is aligned with the function call stack, we can use `plam` as
a stack data structure which is directly synced with the called
functions.


## Other features

Plinth defines macros for making type definitions simple and
consistent. Simple types are defined with `pl_type`:

    pl_type( int8_t, pl_i8 );

This creates the type `pl_i8_t` which is equal to `int8_t`.
Additionally a pointer type `pl_i8_p` is created, which equals to
`int8_t*`.

Struct types are created with `pl_struct`. For example:

    `pl_struct( point )`

will create `pl_struct_s`, `pl_struct_t`, and `pl_struct_p` types. The
`_s` type is the struct itself. `_t` is pointer to the struct, and
`_p` is pointer to the pointer of struct. The base type is considered
to be `_t`, which is aligned with the simple type above. Structs are
typically referenced through a pointer, and therefore `_t` makes sense
as the default type. When struct style objects are collected into a
collection, we need the `_p` type index the collection.

`pl_enum` creates enumeration types. Plinth has it's own boolean type
defines as:

    pl_enum( pl_bool ){ pl_false = 0, pl_true = 1 };

Apart from type definitions, Plinth defines an Universal Interface. It
is universal in the sense, that it can be used in any context,
technically. Universal Interface is useful in situations where we have
to commit to a mechanism, but we don't have much use cases available.
The interface is captured as struct called `pl_ui`, which includes two
members: `fun` and `env`. `fun` is called by the interface master to
communicate with the interface slave. The `env` is used as permanent
context for communication. The `fun` method is declared as:

    typedef pl_none ( *pl_ui_f )( pl_t env, pl_t argi, pl_t argo );

The master passes request towards the slave through `argi` and
receives an response through `argo`. The interface is used by
performing:

    pl_ui_do( ui, argi, argo );

from the master side. While the interface is universal, the
communicating parties are, obviously, required to agree on the
communication content details in advance.


## Plinth API documentation

See Doxygen documentation. Documentation can be created with:

    shell> doxygen .doxygen


## Examples

All functions and their use is visible in tests. Please refer `test`
directory for testcases.


## Building and installing

Plinth is built with the `build.sh` script.

Install is performed with `install.sh`. Please, edit the script for
setting the installation root directory.


## Testing

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
