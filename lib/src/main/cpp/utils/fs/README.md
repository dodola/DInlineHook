# file_system.h

`utils/fs/` is a collection of classes for simplifying the management of
directories and files. It aims to be a lightweight set of wrapper classes, with
the following goals:

* Easy to use
* Platform independent
* Powerful defaults
* Assumes success by default
* Mockable

Together, they make it easy to create, delete, edit, copy, move, overwrite, and
walk over files without worrying about all the finicky details and edge cases
that many of the C system methods normally require you to worry about.

[TOC]

## How to Use

First, create a `DiskFileSystem` instance. A `MemoryFileSystem` is also
available if that better suits your needs, but most use cases require
persistant storage. Then, call `DiskFileSystem::GetDir` to get a directory
handle.

For example, say you have a folder `/usr/docs` that you want to use as a root
folder, and you want to create a file `hello.txt` under the folder
`/usr/docs/tutorials/lesson1`. You can accomplish this with the following
recommended code:

```c++
#include "utils/fs/disk_file_system.h"
DiskFileSystem fs;
auto root = fs.GetOrNewDir("/usr/docs");
auto f = root->NewFile("tutorials/lesson1/hello.txt");
f->OpenForWrite();
f->Append("In this tutorial, ...");
f->Close();
```

We'll cover more `FileSystem` recipes below, but this snippet already
highlights some major themes with this API:

* **Powerful defaults** - You don't have to worry about creating parent
  directories recursively; that is done for you. You don't have to worry about
  checking if a `hello.txt` file is there first; if so, it will just be
  overwritten.
* **Assumes success** - The FileSystem API assumes that this type of code works
  almost all of the time, and therefore that path should be easy to both write
  and read. Functions return boolean values and the system provides various
  state query methods if you need them, however.
* **Easy to use** - Smart pointers are leveraged so you never have to worry
  about managing memory allocation or deletion. _However, you should be careful
  about holding onto file or directory handles if you don't need them anymore._

## Classes

### FileSystem, File, and Dir

In practice, you only need to know about three classes: `FileSystem`, `File`,
and `Dir`.

`FileSystem` is the main entry point for accessing underlying storage data,
which you can query and modify using `Dir` and `File` handles. Note the empasis
on _handles_ here. Just because you have a `File` instance doesn't mean an
actual file exists.

Although a `FileSystem` can create `File` or `Dir` objects from absolute paths,
it is recommended you get a `Dir` handle as soon as possible and do further
modification from there using relative paths.

Use `Dir::GetDir`, `Dir::NewDir`, `Dir::GetOrNewDir`, `Dir::GetFile`,
`Dir::NewFile`, and `Dir::GetAndNewFile` methods. This way, if you move your
root directory later, the rest of your code doesn't need to change:

```c++
auto root = fs.GetDir("/some/root");
auto d = root->GetOrNewDir("a/b/c/d");
auto f = d->NewFile("e/f.txt");
```

### Path

The `Path` class is just the common base-class for `File` and `Dir`. You should
rarely, if ever, need to reference the `Path` class directly, but it is
mentioned here for completion.

## Recipes

The following section shows how you might accomplish various common operations
using the FileSystem API. For brevity, all recipes assume you already created a
handle to a `Dir` assigned to a variable called `root`.

### Read from a File

```c++
std::string contents = root->GetFile("a/b/file.txt")->Contents();
```

### Navigate a directory hierarchy

```c++
auto child1 = root->GetDir("parent/child1");
auto child2 = child1->Up()->GetDir("child2");
```

### Overwrite a file if it exists (but create otherwise)

```c++
auto f = root->NewFile("a/b/file.txt");
f->OpenForWrite();
f->Append("<Your contents here>");
f->Close();
```

### Alternate cout-style

```c++
auto f = root->NewFile("a/b/file.txt");
f->OpenForWrite();
*f << "<Your contents here>" << std::endl;
f->Close();
```

### Append if file exists (but create otherwise)

```c++
auto f = root->GetOrNewFile("a/b/file.txt");
f->OpenForWrite();
f->Append("<Your appended details here>");
f->Close();
```

### Create a new directory (completely deletes any existing!)

```c++
// For this example, /root/a/b/c/d already exists.
assert(root->GetDir("a/b/c/d")->Exists());

root->NewDir("a/b/c");
assert(!root->GetDir("a/b/c/d")->Exists());

```

### Create a file under a subdirectory

```c++
auto dir = root->NewDir("a/b/c");
auto file = dir->NewFile("file.txt");
```

### Copy a file

```c++
auto from = root->GetFile("from/file.txt");
auto to = root->NewFile("to/file.txt");
assert(from->Exists() && to->Exists());
to->OpenForWrite();
to->Append(from->Contents());
to->Close();
```

### Move a file

```c++
auto from = root->GetFile("from/file.txt");
assert(from->Exists());
auto to = root->GetFile("to/file.txt");
from->MoveContentsTo(to);
assert(!from->Exists() && to->Exists());
```

### Delete files that haven't been modified in an hour

```c++
const int EXPIRATION_SECS = 3600; // 1 hour
auto d = root->GetDir("trash");
d->Walk([d](const PathStat &pstat) {
  if (pstat->type() == PathStat::Type::FILE &&
      pstat->modification_age() >= EXPIRATION_SECS) {
    d->GetFile(pstat.rel_path())->Delete();
  }
});
```

### Do a shallow walk (don't visit any subdirectories)

```c++
const int SHALLOW_DEPTH = 1;
root->Walk([](const PathStat &pstat) { ... }, SHALLOW_DEPTH);
```

## Gotchas

### Notes about creating files and directories

If you call `Dir::NewFile` where a _file_ already exists, or if you call
`Dir::NewDir` where a _directory_ already exists, the method will delete what's
currently there.

However, if you call `Dir::NewFile` where a _directory_ exists, or a
`Dir::NewDir` where a _file_ exists, then the call will _not_ overwrite the
target path. This is because this case is assumed to be an unintentional error,
so further progress is blocked. If you find yourself in this situation for some
reason, then explicitly delete the existing item first (with one handle) before
creating a new one (with the other).

```c++
// Hopefully you never need to do this but...
root->GetDir("a/b/c")->Delete();
root->NewFile("a/b/c");
```

_Note that the `Dir::Create` and `File::Create` methods (for creating files and
directories in place) will abort if anything exists at that location, file or
directory. This behavior deviates from the `Dir::NewDir` and `Dir::NewFile`
APIs intentionally, and you are encouraged to prefer `NewDir` and `NewFile`
over in-place creation._

_The idea is that when you have a parent directory, you have a better
understanding of all the sibling files; but when you have a file or directory
handle, you don't necessarily. This implementation choice also allows the
`NewDir` and `NewFile` APIs to safely delegate to the in-place `Create`
methods._

### File read/write mode

A file can only be in read mode OR write mode at any given time. By default, it
is always in read mode except between calls to `File::OpenForWrite` and
`File::Close`.

Calling write methods on a `File` that is not in write mode are no-ops;
simiarly, calling `File::Contents` on a file in write mode will return the
empty string.

Use `File::IsInWriteMode` if you have to worry about this state.

### Touch won't create files

`File::Touch` updates the timestamp of existing files only (unlike its Unix
counterpart, which creates a file if one doesn't already exist). If you want to
mimic such Unix behavior, the pattern is:

```c++
root->GetOrNewFile("touch_me.txt")->Touch();
```

### Walking a directory returns PathStat instances

You might expect an API in `Dir` that looks something like `vector<File>
ListFiles()`. However, when walking a directory, in most cases, you will only
care about some small subset of all the files (say, only files older than a
certain age, or those that end with a certain extension). Therefore, it is not
worth creating `File` and `Dir` instances for _every_ child object, as those
handles are somewhat heavyweight - by design, they are always allocated in the
heap behind a shared pointer.

Therefore, the `Walk` method introduces a more lightweight `PathStat` data
class that provides useful metadata about each child entry, including
information that allows you to create a `File` or `Dir` instance on the spot if
you need.

```c++
dir->Walk([dir](const PathStat &pstat) {
  if (pstat->type() == PathStat::Type::FILE) {
    auto file = dir->GetFile(pstat.rel_path()); // PathStat -> File
    ...
  }
});
```
