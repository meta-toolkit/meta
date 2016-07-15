# MeTA Style Guide

## Code Formatting

- We will follow the code formatting options provided by `clang-format`. Visit
  the file `.clang-format` for our settings.

## Coding Style

- Classes, methods, and variables are all `snake_case`.
- Template parameters are `CamelCase`.
- Nothing is declared in the global namespace.
- No `new`/`delete`; certainly no `malloc()` or `free()`.
- Use exceptions instead of error codes.
- Private members in classes are denoted as `member_`.
- Getters and setters should be

```cpp
type myclass::member() const;
void myclass::member(const type&);
```

- Use `override` (and `final`) wherever possible.
- Prefer brace initialization for classes.
- Use `auto` [almost
  always](http://herbsutter.com/2013/08/12/gotw-94-solution-aaa-style-almost-always-auto/).
- Prefer fixed-width integer types.
- Use range-based for where possible.
- Use `nullptr`, never `NULL` or 0, etc.
- Prefer `enum class` (strongly typed `enum`s).
- Prefer no pointer over `unique_ptr` over `shared_ptr`.
- Do not use `rand()` [deprecated in
  C++14](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3841.pdf).
- Use `#ifndef META_FILE_NAME_H_` for double inclusion guards.
- `#define` kept to a minimum, and ALL_CAPS_SNAKE if used.
- Lines should be no longer than 80 characters

## Documentation

- We will use [Doxygen](http://www.stack.nl/~dimitri/doxygen/) to document our
  source.
- MeTA doxygen settings are in `meta.doxygen.in`.
- In general, the doxygen commenting style we will use is

```cpp
/**
 * Description if necessary. Otherwise can just be in the @return
 * @param thing Capital first letter?
 * @return lowercase for return?
 */
```

- In header files, include the brief LICENSE notice:

```cpp
/**
 * ...
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */
 ```

- In the file comment, use `@file` and (preferrably) `@author`. Of course,
  multiple authors may be listed.

## Source files

- All source files should be in `snake_case`
- Binaries should be `hyphenated-file-names`
