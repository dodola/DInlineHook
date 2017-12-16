/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "utils/fake_clock.h"
#include "utils/fs/memory_file_system.h"

#include <gtest/gtest.h>

using profiler::FakeClock;
using profiler::MemoryFileSystem;
using profiler::Path;
using profiler::PathStat;
using std::make_shared;
using std::set;
using std::string;

TEST(Path, PathStandardizationChecks) {
  EXPECT_EQ(Path::Standardize("/a/b/c/"), "/a/b/c");
  EXPECT_EQ(Path::Standardize("/a/////b//c"), "/a/b/c");
  EXPECT_EQ(Path::Standardize("a/b/c"), "/a/b/c");
  EXPECT_EQ(Path::Standardize("a"), "/a");
  EXPECT_EQ(Path::Standardize("/"), "/");
  EXPECT_EQ(Path::Standardize(""), "/");
}

TEST(Path, StripLastChecks) {
  EXPECT_EQ(Path::StripLast("/a/b/c"), "/a/b");
  EXPECT_EQ(Path::StripLast("/a/b"), "/a");
  EXPECT_EQ(Path::StripLast("/a/b.txt"), "/a");
  EXPECT_EQ(Path::StripLast("/a"), "/");
  EXPECT_EQ(Path::StripLast("/"), "/");
}

TEST(FileSystem, CanGetAndCreateDir) {
  MemoryFileSystem fs;
  auto root = fs.GetDir("/mock/root");

  EXPECT_FALSE(root->Exists());
  root->Create();
  EXPECT_TRUE(root->Exists());
}

TEST(FileSystem, CreatingSubdirCreatesParentDirsAutomatically) {
  MemoryFileSystem fs;

  auto d = fs.GetDir("/a/b/c/d");
  d->Create();
  EXPECT_TRUE(d->Exists());
  EXPECT_TRUE(fs.GetDir("/a")->Exists());
}

TEST(FileSystem, CanCreateSubdirFromParentDir) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto subdir = root->NewDir("subdir");
  EXPECT_TRUE(subdir->Exists());
  EXPECT_EQ(subdir->path(), "/mock/root/subdir");
}

TEST(FileSystem, UpReturnsExpectedParent) {
  MemoryFileSystem fs;
  auto dir = fs.GetDir("/a/b/c");

  EXPECT_EQ(dir->path(), "/a/b/c");
  EXPECT_EQ(dir->Up()->path(), "/a/b");
  EXPECT_EQ(dir->Up()->Up()->path(), "/a");
  EXPECT_EQ(dir->Up()->Up()->Up()->path(), "/");
  // Can't go past root
  EXPECT_EQ(dir->Up()->Up()->Up()->Up()->path(), "/");
}

TEST(FileSystem, CanCreateFileFromParentDir) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto file = root->NewFile("file.txt");
  EXPECT_TRUE(file->Exists());
  EXPECT_EQ(file->name(), "file.txt");
}

TEST(FileSystem, CanCreateFileInPlace) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto file = root->GetFile("file.txt");
  EXPECT_FALSE(file->Exists());
  file->Create();
  EXPECT_TRUE(file->Exists());
}

TEST(FileSystem, AllParentDirectoriesAreCreatedForNewDir) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");
  auto subdirs = root->NewDir("sub1/sub2/sub3");

  EXPECT_TRUE(root->GetDir("sub1")->Exists());
  EXPECT_TRUE(root->GetDir("sub1/sub2")->Exists());
  EXPECT_TRUE(root->GetDir("sub1/sub2/sub3")->Exists());
}

TEST(FileSystem, AllParentDirectoriesAreCreatedForNewFile) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");
  root->NewFile("sub1/sub2/file.txt");

  EXPECT_TRUE(root->GetDir("sub1")->Exists());
  EXPECT_TRUE(root->GetDir("sub1/sub2")->Exists());
  EXPECT_TRUE(root->GetFile("sub1/sub2/file.txt")->Exists());
}

TEST(FileSystem, CallingNewDirOverExistingDirDeletesIt) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto c = root->NewDir("a/b/c");
  EXPECT_TRUE(c->Exists());

  auto a = root->NewDir("a");

  EXPECT_TRUE(a->Exists());
  EXPECT_FALSE(c->Exists());
}

TEST(FileSystem, CreatingDirInPlaceOverExistingFails) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto c = root->NewDir("a/b/c");
  auto a = root->GetDir("a");
  EXPECT_TRUE(a->Exists());
  EXPECT_TRUE(c->Exists());

  EXPECT_FALSE(a->Create());
}

TEST(FileSystem, CantCreateFileIfDirAlreadyExists) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto dir = root->NewDir("a/b/c");
  auto file = root->NewFile("a/b/c");

  EXPECT_TRUE(dir->Exists());
  EXPECT_FALSE(file->Exists());
}

TEST(FileSystem, CantCreateDirIfFileAlreadyExists) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto file = root->NewFile("a/b/c");
  auto dir = root->NewDir("a/b/c");

  EXPECT_TRUE(file->Exists());
  EXPECT_FALSE(dir->Exists());
}

TEST(FileSystem, DotDotDirectoriesAreNotAllowed) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto subdir = root->NewDir("../invalid");
  EXPECT_FALSE(subdir->Exists());
}

TEST(FileSystem, DeletingDirectoryDeletesChildren) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto d_parent = root->NewDir("deleteme");
  auto f = d_parent->NewFile("a/b/c/d.txt");
  auto d_child = d_parent->GetDir("a/b");

  EXPECT_TRUE(d_parent->Exists());
  EXPECT_TRUE(d_child->Exists());
  EXPECT_TRUE(f->Exists());

  d_parent->Delete();

  EXPECT_FALSE(d_parent->Exists());
  EXPECT_FALSE(d_child->Exists());
  EXPECT_FALSE(f->Exists());
}

TEST(FileSystem, ConstAccessAllowsReadOnlyView) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  root->NewFile("a/b/c/d.txt");
  root->NewDir("a/b/c2");

  const auto const_root = root;
  EXPECT_TRUE(const_root->Exists());
  EXPECT_TRUE(const_root->GetDir("a/b/c")->Exists());
  EXPECT_FALSE(const_root->GetDir("1/2/3")->Exists());
  EXPECT_TRUE(const_root->GetFile("a/b/c/d.txt")->Exists());
}

TEST(FileSystem, TouchUpdatesModificationAge) {
  auto clock = make_shared<FakeClock>(100);
  MemoryFileSystem fs(clock);
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("file.txt");

  EXPECT_EQ(f->GetModificationAge(), 0);

  clock->SetCurrentTime(200);
  EXPECT_EQ(f->GetModificationAge(), 100);

  f->Touch();
  EXPECT_EQ(f->GetModificationAge(), 0);
}

TEST(FileSystem, NonExistantFilesAlwaysHaveZeroModificationAge) {
  auto clock = make_shared<FakeClock>(100);
  MemoryFileSystem fs(clock);
  auto root = fs.NewDir("/mock/root");

  auto f = root->GetFile("file.txt");
  EXPECT_EQ(f->GetModificationAge(), 0);
  clock->SetCurrentTime(200);
  EXPECT_EQ(f->GetModificationAge(), 0);
}

TEST(FileSystem, WalkDirectoriesWorks) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto d = root->NewDir("d");
  d->NewFile("f1");
  d->NewFile("f2");
  d->NewFile("a/b/c/f3");

  int path_count = 0;
  d->Walk([&path_count](const PathStat &pstat) { ++path_count; });
  EXPECT_EQ(path_count, 6);
}

TEST(FileSystem, ConstWalkDirectoriesWorks) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto d = root->NewDir("d");
  d->NewFile("f1");
  d->NewFile("f2");
  d->NewFile("a/b/c/f3");

  const auto const_root = root;
  auto const_d = const_root->GetDir("d");

  int path_count = 0;
  const_d->Walk([&path_count](const PathStat &pstat) { ++path_count; });
  EXPECT_EQ(path_count, 6);
}

TEST(FileSystem, WalkDirectoriesReportsCorrectStats) {
  auto clock = make_shared<FakeClock>(100);
  MemoryFileSystem fs(clock);
  auto root = fs.NewDir("/mock/root");

  auto b = root->NewDir("a/b");
  root->NewFile("a/b/c/d/e/f.txt");
  clock->SetCurrentTime(350);

  int file_count = 0;
  b->Walk([&file_count](const PathStat &pstat) {
    if (pstat.type() == PathStat::Type::FILE) {
      ++file_count;
      EXPECT_EQ(pstat.rel_path(), "c/d/e/f.txt");
      EXPECT_EQ(pstat.full_path(), "/mock/root/a/b/c/d/e/f.txt");
      EXPECT_EQ(pstat.modification_age(), 250);
    }
  });

  EXPECT_EQ(file_count, 1);
}

TEST(FileSystem, WalkDirectoriesWithMaxDepthWorks) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  root->NewFile("file01");
  root->NewFile("dir1/file11");
  root->NewFile("dir1/file12");
  root->NewFile("dir1/dir2/file21");
  root->NewFile("dir1/dir2/file22");
  root->NewFile("dir1/dir2/file23");
  root->NewFile("dir1/dir2/dir3/file31");
  root->NewFile("dir1/dir2/dir3/file32");
  root->NewFile("dir1/dir2/dir3/file33");
  root->NewFile("dir1/dir2/dir3/file34");
  root->NewFile("dir1/dir2/dir4/file41");
  root->NewFile("dir1/dir2/dir4/file42");
  root->NewFile("dir1/dir2/dir4/file43");
  root->NewFile("dir1/dir2/dir4/file44");
  root->NewFile("dir1/dir2/dir4/file45");

  {
    set<string> files;
    root->Walk(
        [&files](const PathStat &pstat) {
          if (pstat.type() == PathStat::Type::FILE) {
            files.insert(pstat.rel_path());
          }
        },
        3);

    EXPECT_TRUE(files.find("file01") != files.end());
    EXPECT_TRUE(files.find("dir1/file11") != files.end());
    EXPECT_TRUE(files.find("dir1/file12") != files.end());
    EXPECT_TRUE(files.find("dir1/dir2/file21") != files.end());
    EXPECT_TRUE(files.find("dir1/dir2/file22") != files.end());
    EXPECT_TRUE(files.find("dir1/dir2/file23") != files.end());
    EXPECT_EQ(files.size(), 6u);
  }

  {
    set<string> files;
    root->Walk(
        [&files](const PathStat &pstat) {
          if (pstat.type() == PathStat::Type::FILE) {
            files.insert(pstat.rel_path());
          }
        },
        1);

    EXPECT_TRUE(files.find("file01") != files.end());
    EXPECT_EQ(files.size(), 1u);
  }

  {
    set<string> files;
    root->Walk(
        [&files](const PathStat &pstat) {
          if (pstat.type() == PathStat::Type::FILE) {
            files.insert(pstat.rel_path());
          }
        },
        0);

    EXPECT_EQ(files.size(), 0u);
  }
}

TEST(FileSystem, CanWriteToFile) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("test.txt");
  EXPECT_EQ(f->Contents(), "");
  f->OpenForWrite();
  f->Append("Hello");
  f->Append(" World");
  f->Close();
  EXPECT_EQ(f->Contents(), "Hello World");
}

TEST(FileSystem, CannotWriteToFileNotInWriteMode) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("test.txt");
  EXPECT_EQ(f->Contents(), "");
  f->Append("Hello");
  EXPECT_EQ(f->Contents(), "");
  f->Append(" World");
}

TEST(FileSystem, CannotReadFromFileInWriteMode) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("test.txt");
  f->OpenForWrite();
  f->Append("Hello World");
  EXPECT_EQ(f->Contents(), "");
  f->Close();
  EXPECT_EQ(f->Contents(), "Hello World");
}

TEST(FileSystem, CanAppendUsingShiftOperator) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("test.txt");
  f->OpenForWrite();
  *f << "123 * 456 == " << 123 * 456 << '\n';
  f->Close();

  EXPECT_EQ(f->Contents(), "123 * 456 == 56088\n");
}

TEST(FileSystem, DeletingFileRemovesContents) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("test.txt");
  f->OpenForWrite();
  f->Append("Goodbye");
  f->Close();
  EXPECT_EQ(f->Contents(), "Goodbye");
  f->Delete();
  EXPECT_EQ(f->Contents(), "");
}

TEST(FileSystem, DeletingDirectoryUnderFileRemovesContents) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("a/b/c/d/test.txt");
  f->OpenForWrite();
  f->Append("Goodbye");
  f->Close();
  EXPECT_EQ(f->Contents(), "Goodbye");
  root->Delete();
  EXPECT_EQ(f->Contents(), "");
}

TEST(FileSystem, WritesToNonExistantFileAreIgnored) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->GetFile("test.txt");
  EXPECT_FALSE(f->Exists());
  f->OpenForWrite();
  f->Append("Hello World");
  f->Close();
  EXPECT_EQ(f->Contents(), "");
}

TEST(FileSystem, DeletingFileClosesIt) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("test.txt");
  f->OpenForWrite();

  EXPECT_TRUE(f->IsOpenForWrite());

  f.reset();  // Calls ~File()
  f = root->GetFile("text.txt");
  EXPECT_FALSE(f->IsOpenForWrite());
}

TEST(FileSystem, CreatingFileInPlaceOverExistingFileFails) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("file.txt");
  f->OpenForWrite();
  f->Append("Hello World");
  f->Close();
  EXPECT_EQ(f->Contents(), "Hello World");

  EXPECT_FALSE(f->Create());
  EXPECT_EQ(f->Contents(), "Hello World");
}

TEST(FileSystem, CallingNewFileOverExistingFileDeletesIt) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f = root->NewFile("file.txt");
  f->OpenForWrite();
  f->Append("Hello World");
  f->Close();
  EXPECT_EQ(f->Contents(), "Hello World");

  root->NewFile("file.txt");
  EXPECT_EQ(f->Contents(), "");
}

TEST(FileSystem, MovingFileWorks) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f1 = root->NewFile("f1.txt");
  auto f2 = root->GetFile("f2.txt");
  f1->OpenForWrite();
  f1->Append("Test contents");
  f1->Close();

  EXPECT_TRUE(f1->Exists());
  EXPECT_EQ(f1->Contents(), "Test contents");
  EXPECT_FALSE(f2->Exists());

  f1->MoveContentsTo(f2);
  EXPECT_FALSE(f1->Exists());
  EXPECT_TRUE(f2->Exists());
  EXPECT_EQ(f2->Contents(), "Test contents");
}

TEST(FileSystem, MovingFileFailsIfSrcFileDoesntExist) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f1 = root->GetFile("f1.txt");
  auto f2 = root->NewFile("f2.txt");
  f2->OpenForWrite();
  f2->Append("Not overwritten");
  f2->Close();

  EXPECT_FALSE(f1->Exists());
  EXPECT_TRUE(f2->Exists());

  f1->MoveContentsTo(f2);
  EXPECT_FALSE(f1->Exists());
  EXPECT_TRUE(f2->Exists());
  EXPECT_EQ(f2->Contents(), "Not overwritten");
}

TEST(FileSystem, MovingFileFailsIfSrcIsInWriteMode) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f1 = root->NewFile("f1.txt");
  auto f2 = root->GetFile("f2.txt");
  f1->OpenForWrite();
  f1->Append("Not moved");
  f1->Close();

  f1->OpenForWrite();
  f1->MoveContentsTo(f2);
  f1->Close();
  EXPECT_TRUE(f1->Exists());
  EXPECT_EQ(f1->Contents(), "Not moved");
  EXPECT_FALSE(f2->Exists());
}

TEST(FileSystem, MovingFileFailsIfDestIsInWriteMode) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f1 = root->NewFile("f1.txt");
  auto f2 = root->NewFile("f2.txt");
  f1->OpenForWrite();
  f1->Append("Not moved");
  f1->Close();

  f2->OpenForWrite();
  f1->MoveContentsTo(f2);
  f2->Close();

  EXPECT_TRUE(f1->Exists());
  EXPECT_EQ(f1->Contents(), "Not moved");
  EXPECT_TRUE(f2->Exists());
  EXPECT_EQ(f2->Contents(), "");
}

TEST(FileSystem, MovingFileIsNoOpIfFileIsMovedInPlace) {
  MemoryFileSystem fs;
  auto root = fs.NewDir("/mock/root");

  auto f1 = root->NewFile("f1.txt");
  auto f2 = root->GetFile("f1.txt");
  f1->OpenForWrite();
  f1->Append("Test contents");
  f1->Close();

  EXPECT_TRUE(f1->Exists());
  EXPECT_EQ(f1->Contents(), "Test contents");
  EXPECT_TRUE(f2->Exists());
  EXPECT_EQ(f2->Contents(), "Test contents");

  f1->MoveContentsTo(f2);
  EXPECT_TRUE(f1->Exists());
  EXPECT_EQ(f1->Contents(), "Test contents");
  EXPECT_TRUE(f2->Exists());
  EXPECT_EQ(f2->Contents(), "Test contents");
}
