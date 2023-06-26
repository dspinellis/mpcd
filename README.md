![Build Status](https://img.shields.io/github/actions/workflow/status/dspinellis/mpld/main.yml?branch=main)

# mpcd: modular performant clone detector

Read from the standard input tokenized files and output sets of clones.

The _mpcd_ modular performant clone detector is yet another attempt to
implement a code clone detector.
The rationale for building it was to address shortcomings of existing systems
in the areas of flexibility and performance.

On the flexibility front _mpcd_ splits the functionality often found
in other systems into three parts: locating the files to examine,
extracting elements that can be recognized as clones (tokenization), and
identifying clones.
The _mpcd_ tool is structured so as to allow the first two parts to be
performed independently of it.
Specifically, in an _mpcd_ workflow file location is handled by external
tools, such as _find_(1) or _git-ls-tree_(1),
while tokenization is also handled by an another external tool,
such as [tokenizer](https://github.com/dspinellis/tokenizer).

To maximize performance _mpcd_ is implemented in C++ with
statically typed objects structured to avoid the memory overheads of boxing,
and
data structures carefully chosen and used to achieve high and scalable
runtime performance.


## Build

```
cd src
make
```

## Test
Ensure [CppUnit](https://en.wikipedia.org/wiki/CppUnit) is installed.
Depending on your environment, you may also need to pass its installation
directory prefixes to _make_ through the command line arguments.
For example, under macOS pass
`ADDCXXFLAGS='-I /opt/homebrew/include' ADDLDFLAGS='-L /opt/homebrew/lib'`
as arguments to _make_.

```
cd src
make test
```

## Install

```
cd src
sudo make install
```

## Run

The clone detector's input is a stream of file identifiers
(e.g. file paths) prefixed with `F`, followed by each file's
tokens for each line in the range 0–699.
These can be obtained by applying the
[tokenizer](https://github.com/dspinellis/tokenizer) program on each file.

### Example

The following example identifies Type 1 (exact) clones in all Java files
located in the `src` directory.
The _tokenizer_'s
`-f` option is used to identify each output file and
the `-g` option to process all identifiers in the same (global) scope.

```sh
# Obtain list of Java files in the src directory
find src -type f -name '*.java' |

# Tokenize files in paths coming from stdin, using the same scope for all ids
tokenizer -l Java -o line -l - -g -f |

# Identify clones
mpcd >results.txt
```

The following example identifies Type 2 (near) clones
in the Linux kernel v6.3 files located in the `kernel` directory,
by retrieving the list of files and their contents from a bare Git
repository.
The _okenizer_'s
`-c` option is used to output the same token value for all identifiers.
Adjust clone size to 40 tokens and generate verbose output while processing.

```sh
# Obtain list of files in the bare repository
git --git-dir linux.git ls-tree -r --name-only v6.3 -- kernel |

# Use only C files
grep '\.[ch]$' |

# For each file
while read file; do
  # Identify the file being output
  echo "F$file"

  # Obtain and tokenize the file's contents
  git --git-dir linux.git show "v6.3:$file" |
  tokenizer -l C -o line -c
done |

# Identify clones
mpcd -n 40
```

## Reference manual
You can read the command's Unix manual page through [this link](https://dspinellis.github.io/manview/?src=https%3A%2F%2Fraw.githubusercontent.com%2Fdspinellis%2Fmpcd%2Fmaster%2Fsrc%2Fmpcd.1&name=mpcd(1)&link=https%3A%2F%2Fgithub.com%2Fdspinellis%2mpcd).

