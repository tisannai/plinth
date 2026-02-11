# Plinth - Base layer library

Plinth is a base layer library for small to midsize C programming
projects. Most of the Plinth features are for memory allocations and
storage, but it also provides facilities for string handling and basic
type definitions.

Plinth allows the memory allocators to be nested, i.e. arrangements
where the nested allocator allocates from the host (base) allocator.
Nesting provides flexible and efficient memory allocations.
Pre-existing allocations can be used for all allocator types. The
allocation can be from stack or from another allocator. Pre-existing
allocations are without Debt and other allocations are with Debt.

```
        plam   +++++------           nested
                \       /
                 \     /
                  \   /
                   \ /
        plbm        ++++ <-> +++-    host
```

Allocators are used through handles, which are pointers to Allocator
Descriptors. All allocators allow persistent references (i.e. stable
pointers), except the Continuous Memory Allocator (details below).
Allocator nesting would not be possible without persistent references.

See Doxygen docs and `plinth.h` for details about Plinth API. Also
consult the test directory for usage examples.


## Basic Memory Allocation

Plinth defines basic heap memory allocation functions:
`pl_alloc_memory()`, `pl_free_memory()`, and `pl_realloc_memory()`.
These are equal to `malloc()`, `free()`, and `realloc()`. Additionally
there is `pl_alloc_string()` for string duplication and
`pl_format_string()` for formatting strings to heap allocations. Both
`pl_alloc_string()` and `pl_format_string()` results must be
deallocated with `pl_free_memory()`.


## Arena Memory Allocator

`plam` is the fundamental memory allocator in Plinth. It is an Arena
type allocator. `plam` consists of one or more Nodes, which are
chained together as a doubly linked list. Each allocation from `plam`
is a continuous chunk of memory from a Node. When the memory from one
Node is exhausted, a new Node is allocated and added to the chain.
Memory can be deallocated back to `plam`, but only in the reverse
order it was reserved, and each deallocation must be annotated with
its size. `plam` does not keep track of the detailed allocation
history.

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

`plam` can be created from:

* Heap, with `plam_new()` (or `plam_empty()`). All Nodes are heap
  allocated and must be freed (Debt).

* Pre-allocation, with `plam_use()`, `plam_use_plam()`, or
  `plam_use_plbm()`. The first Node has no Debt and the rest (if any)
  have Debt.

* Another `plam`, with `plam_into_plam()` (or
  `plam_empty_into_plam()`). All Nodes are from host and can be freed,
  if beneficial.

* `plbm`, with `plam_into_plbm()` (or `plam_empty_into_plbm()`). All
  Nodes are from host and can be freed, if beneficial.

The `empty` versions cause no initial allocations and can be used for
lazy behavior.

An allocation is performed with `plam_get()`. When the current Node
runs out of memory, a new Node is allocated from heap or from host.
User can also deallocate (put back) allocations with `plam_put()`. The
deallocations must occur in reverse order to the allocations and they
have to be annotated with the corresponding allocation size. When
deallocations progress to the previous Nodes (towards left), then new
allocations will be returned from the empty Nodes (towards right),
because Nodes themselves are not freed.

A nested `plam` allocates Node memory from host with `plam_get()` and
deallocates with `plam_put()`. When a `plam` is hosted by another
`plam`, the nested `plam` may free itself only if the are no other
allocations from the host during the lifetime of the nested `plam`.
Otherwise, there is no quarantee of deallocations in reverse order to
the host. When a `plam` is hosted by a `plbm`, there are no
limitations on the order of deallocations and other allocations from
the host.

The complete chain of Nodes is deallocated with `plam_del()`.
`plam_del()` does real deallocation only when the `plam` Node has
Debt. `plam_del()` can be safely called with all the allocation
scenarios.


## Block Memory Allocator

`plbm` is a Block Memory Allocator. It allocates "small" Blocks of
memory with fixed sizes. The Blocks can be allocated and deallocated
in any order. Under the hood, the deallocated Blocks are maintained in
a linked list. Head points to the start of the list. Each Block is a
continuous chunk of memory, but memory between Blocks is not
guaranteed to be continuous. However, back-to-back allocations (no
deallocations in between) from a particular Node are continuous.
`plbm` shares the same Node structure as `plam`. Each Node in `plbm`
contains usually multiple Blocks, but one is the minimum.

```
           ..       .-.
           v|       | v
        #++-- <-> #+-+-
           |^       ^ |
           o'-------|-'
                    head
```

`plbm` can be created from:

* Heap, with `plbm_new()` (or `plbm_empty()`). All Nodes are heap
  allocated and must be freed (Debt).

* Pre-allocation, with `plbm_use()`, `plbm_use_plam()`, or
  `plbm_use_plbm()`. First Node has no Debt and the rest (if any) have
  Debt.

* `plam`, with `plbm_into_plam()` (or `plbm_empty_into_plam()`). All
  Nodes are from host and can be freed, if beneficial.

* Another `plbm`, with `plbm_into_plbm()` (or
  `plbm_empty_into_plbm()`). All Nodes are from host and can be freed,
  if beneficial.

The `empty` versions cause no initial allocations and can be used for
lazy behavior.

A Block of memory is allocated with `plbm_get()` and deallocated with
`plbm_put()`. Every deallocated Block becomes part of the
free-block-chain, which is used as priority for every new Block
allocation. When there are no Blocks in the chain or available Blocks
in the current Node, a new Node is allocated and a Block is returned
from the new Node. The Blocks are returned in order from new Nodes,
and therefore it is possible for the user to get continuous memory
from `plbm`.

The complete chain of Nodes is deallocated with `plbm_del()`.
`plbm_del()` does real deallocation only when the `plbm` Node has
Debt. `plbm_del()` can be safely called with all the allocation
scenarios.


## Continuous Memory Allocator

`plcm` is a Continuous Memory Allocator. `plcm` is used when a
continuous chunk of memory is required. This is typical for strings
and other arrays of items.

```
           plcm_s    data
          /         /    used
         /         /    /   ,size
        #    =>   +++++-----
```

`plcm` can be created from:

* Heap, with `plcm_new()`. Memory is heap allocated and must be
  freed (Debt).

* Pre-allocation, with `plcm_use()`, `plcm_use_plam()`, or
  `plcm_use_plbm()`. At first memory has no Debt, but if required
  allocation size exceeds the initial capacity, memory is allocated
  from heap and becomes Debt.

`plcm` is created with an initial allocation size. The user makes
allocations from the initial allocation, until it runs out. Regardless
of the initial allocation storage (Debt or not), the next allocation
is always from heap (with Debt). When the allocation is renewed, the
location of the memory is likely to change, and therefore all direct
references to memory from `plcm` become invalid. For this reason,
references to allocations are by default offsets to the base address.
The offsets are called Positions. When Position is used, it is added
to the current base address and therefore the resulting reference is
up-to-date and valid.

Allocations can be made with `plcm_get_pos()`, where the return value
is a Position to the allocator memory. Other possibility is
`plcm_get_ref()`, where the return value is a pointer to the allocated
location, and has the relocation limitations mentioned above. User may
also allocate and store the value in a single action with the
`plcm_store()` function.

Deallocation (put back) is possible with `plcm_put()`. The
deallocations must occur in reverse order to the allocations and they
have to be annotated with the corresponding allocation size.

If the continuous data storage requires a terminating value,
`plcm_terminate()` can be used. For example, a NULL terminated array
can be done with `plcm_terminate()`.

`plcm` may contain multiple allocations, but typically there is only
one allocation, with changing size. The only allocation that can
change its size in `plcm` is the last, and obviously the only
allocation is also the last.


## Unified Memory Allocator

`plum` is the Unified Memory Allocator in Plinth. It provides an
unified interface to the other allocators in Plinth, including the
Basic Memory Allocation (BMA). The underlying, actual, allocator is
called a host.

`plum` is created with `plum_use()`. It takes `plum` handle, the
allocator type and the allocator handle (NULL for BMA) as arguments.

After creation, `plum` allocation functions are ready to be used.
`plum_get()` gets allocation from the registered host depending on the
host type. `plum_put()` is used for returning the allocation back.
`plum_update()` is used to resize the current allocation.

`plum` does not try to overcome all the limitations that the different
hosts expose. BMA host has no surprises. For `plum_get()`, all hosts
return the allocation as is, except with `plbm` host the maximum
allocation size is obviously the size of the `plbm` block.

For `plam` host, `plum_put()` is able to fully utilize the returned
memory if returns are performed in reverse order to the allocations.
The same applies to a `plcm`. For a `plbm` host, the order of returns
have no limitations.

With `plum_update()`, the data in the allocation is retained. If the
new allocation starts from the same location as the current, no data
needs copying. If the current location can't be used, the new location
is taken into use and current data is copied over. The amount of
copied data is based on the smaller size between current and new size.


## String Storage

`plss` provides a set of functions for storing string data to a
`plcm`. User can append to existing strings with `plss_append()`,
`plss_append_string()`, `plss_append_char()`, or with
`plss_format_string()`.

```
           plcm_s    string
          /         /    used
         /         /    /   ,size
        #    =>   +++++o----
                        \
                         null
```

The existing string can be replaced with `plss_set()` or
`plss_reformat_string()`.

The stored string content is retrieved with `plss_string()` and length
with `plss_length()`.

Each `plss` storage function ensures that the stored string is NULL
terminated and can be therefore used directly with C library
functions.


## String References

`plsr` provides a set of functions for operating with immutable string
references. A string reference is a pair of string content and string
length. String references may point to any location in a C-string,
since it contains the explicit length and does not require the NULL
terminator.

```
            content,length
           /
        .-----.
        ++++++++++++o-----
               '---'
                  \
                   content,length
```

String reference provides an efficient way of examining and
constructing strings. String references are used by value, i.e. the
`plsr` struct (`plsr_s`) is copied from the caller to the callee. This
allows function call nesting and in overall, more functional style of
programming.

`plsr` can be created from C string with `plsr_from_string()` or if
the length is already known with `plsr_from_string_and_length()`.
`plsr_string()` returns the content and `plsr_length()` returns the
length. `plsr_compare()` and `plsr_compare_n()` can be used for simple
same-or-not-same comparison.

`plsr_null()` returns a `plsr` value that is semantically same as a
string NULL pointer. `plsr_is_null()` is used to test if the `plsr` is
NULL or not.


## Memory Allocation Strategies

Program performance is often limited by memory bandwidth and latency.
We want to get the data from the nearest possible cache level, because
it is the fastest. This is assisted by minimizing the amount of data
in use, reuse of same addresses, and laying out the data to
consecutive addresses (arrays).

Each allocated data has size and lifetime. The easiest case for
allocations is when we know the maximum or exact data size and the
lifetime, in advance. The most difficult case is when we don't know
the data size and the lifetime might vary. The efficiency of memory
allocation solutions is dependent on predictability.

The table below provides a quick summary of allocation options. The
options are not exclusive and therefore other options might be used as
well. Use this as a starting point for detailed considerations. The
first option listed is likely the best fit. Memory originates from
either a pre-allocation (stack or another allocator) or from the heap.

| Size known | Lifetime known | Continuous | Allocator                       |
|:-----------|:---------------|:-----------|:--------------------------------|
| yes        | yes            | yes        | `plam` `plbm` `plcm` (pre/heap) |
| yes        | yes            | no         | `plbm` `plam` (pre/heap)        |
| yes        | no             | yes        | `plam` `plbm` `plcm` (heap)     |
| yes        | no             | no         | `plbm` `plam` (heap)            |
| no         | yes            | yes        | `plcm` (pre/heap)               |
| no         | yes            | no         | `plam` (pre/heap)               |
| no         | no             | yes        | `plcm` (heap/pre)               |
| no         | no             | no         | `plam` (heap)                   |

Strings, and other array types, need to be allocated to a continuous
chunk of memory. This is possible with `plam`, when we know the size
(or at least the maximum size) in advance. For unknown sizes, we
should use `plcm`.

Independent objects, or in general fixes size items, can be allocated
with `plbm`. `plbm` supports efficient and convenient
allocation/deallocation sequences, since all allocations can be
deallocated in any order.

If the there is significant amount of variation in data object sizes,
the simplest option for allocations is `plam`. It has, in practice, no
limitations on object sizes and their variation.

Stack allocations are suitable for objects that are short lived. Their
lifetime is within a function. Stack allocations should be used as
much as possible, because they are very efficient. However, stack
allocations should be reserved for only small allocations that can be
discarded at function exit. Recursive functions should not normally
make much stack allocations, since every recursion adds up and might
eventually cause a stack overflow.

Array of memory from stack can be especially useful for the `plcm`
allocator. Quite often, a string is to be build within a function. The
user can reserve an array of chars from stack as memory and use it
through `plcm_use()`. The array size should be selected so that,
typically the built string fits into the stack array. In the rare
occasion when the stack memory is not enough, the allocation is moved
to heap automatically by `plcm`. The typical case remains fast, since
no heap allocation is performed. User may always call `plcm_del()`
near the function exit regardless of the actual allocation type, since
`plcm` keeps track of the allocation type.

In general, allocations should be performed as early as possible. If
the function that uses the allocated memory does not have to know
where the memory originated, we have a less coupled function. Less
coupled functions are more reusable. There are less responsibilities
for the function. Additionally, the called function should not
deallocate the memory, since all the relevant information about the
allocation exist in the caller.

When data lifetime exceeds the lifetime of the allocating function,
the data must be allocated from heap. Also, when the size of the data
is large, we should use heap. Direct heap allocations with
`pl_alloc_memory()` are not preferred. A common strategy is to first
allocate large amount of memory with `plam` (the host allocator) and
then create nested allocators into the host. If the allocated objects
are independent, we can use `plbm`. If we need a continuous array, we
can use `plcm`. The initial size of `plcm` should be large enough to
cover the typical use case. However, if `plcm` capacity is exhausted,
the allocation is moved to heap. In this case the initial `plcm`
allocation becomes overhead in the host `plam`. If this is an issue,
the `plcm` allocation type can be identified with `plcm_debt()` and
the initial allocation can be deallocated.

Allocators can be created to be initially empty. This is beneficial
when there is a possibility that allocator is not used at all. The
empty allocator will perform real allocations only when the first
allocation is actually requested.

For moderate object lifetimes, we can use `plam` as the base
allocator. If there is a lot of variation in the object lifetimes, we
might benefit from frequent deallocations. In this case, we could have
a `plbm` with large Block sizes and nest `plam` allocators into
Blocks. We can make multiple subsequent (small) allocations to `plam`,
and when the objects can be discarded, we deallocate the Block back to
the hosting `plbm`. Another set of objects might have a longer
lifetime and they could be allocated to another `plam`, contained in a
another Block.

When data lifetime is equal to the program lifetime, the allocation
strategy becomes simple. It really doesn't matter how memory is
allocated, because it is a one-time operation.

Large objects typically have long lifetimes. Midsized and small
objects can have either short or long lifetimes. The combination of
short and long lifetime objects, with varying sizes, is challenging.
These scenarios require that at least some allocation are performed
using the general purpose `pl_alloc_memory()` and `pl_free_memory()`.


## Other features

Plinth defines macros for making type definitions simple and
consistent. Simple types are defined with `pl_type`:

    pl_type( int8_t, pl_i8 );

This creates the type `pl_i8_t` which is equal to `int8_t`.
Additionally a pointer type `pl_i8_p` is created, which equals to
`int8_t*`.

Struct types are created with `pl_struct`. For example:

    pl_struct( point ) { ... };

will create `point_s`, `point_t`, and `point_p` types. The `_s` type
is the struct itself. `_t` is the pointer to the struct, and `_p` is
the pointer to the pointer of struct. The base type is considered to
be `_t`, which is consistent with the simple types above. Structs are
typically referenced through a pointer, and therefore `_t` makes sense
as the base type. When struct style objects are collected into a
collection, we need the `_p` type to use the collection.

`pl_enum` creates enumeration types. Plinth has it's own boolean type
defines as:

    pl_enum( pl_bool ){ pl_false = 0, pl_true = 1 };

Apart from type definitions, Plinth defines an Universal Interface. It
is universal in the sense, that it can be used in any context,
technically. Universal Interface is useful in situations where we have
to commit to a mechanism, but we don't have much use cases available.
The interface is captured as struct called `pl_ui`, which includes two
members: `fun` and `env`. `pl_ui` is initialized with `pl_ui_init`.
`fun` is called by the interface master to communicate with the
interface slave. The `env` is used as permanent context for
communication. The `fun` method is declared as:

    typedef pl_none ( *pl_ui_f )( pl_t env, pl_t argi, pl_t argo );

The master passes requests towards the slave through `argi` and
receives responses through `argo`. The interface is used by
performing:

    pl_ui_do( ui, argi, argo );

from the master side. While the interface is universal, the
communicating parties are, obviously, required to agree on the
communication content details in advance. The opaque datastructure
referenced by `argi`, contains typically an ID field as the first
struct field (follow by actual payload). The ID field can be used to
identify the type of provided content.


## Plinth API documentation

See Doxygen documentation. Documentation can be created with:

    shell> doxygen .doxygen


## Examples

All functions and their use is visible in tests. Please refer `test`
directory for testcases.


## Building and installing

Plinth is built with the `do-build` script.

Install is performed with `do-install`. Please, edit the script for
setting the installation root directory.


## Testing

Ceedling based flow is in use:

    shell> ceedling

Testing:

    shell> ceedling test:all

User defines can be placed into `project.yml`. Please refer to
Ceedling documentation for details.


## Ceedling

Standard Ceedling files are not in GIT. These can be added by
executing:

    shell> ceedling new plinth

in the directory above Plinth. Ceedling prompts for file overwrites.
You should answer NO in order to use the customized files.
