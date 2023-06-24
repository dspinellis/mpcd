# mpcd: modular performant clone detector

Read from the standard input tokenized files and output
sets of clones.

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

The following example identifies clones for all Java files
located in the `src` directory.

```sh
# Obtain list of Java files in the src directory
find src -type f -name '*.java' -print0 |
# For each file
while IFS= read -r -d '' file; do
  echo "F$file"
  # Tokenize the file's contents
  tokenizer -l Java -o line -t N "$file"
done |
# Identify clones
mpcd >results.txt
```

The following example identifies clones in the Linux kernel v6.3
files located in the `kernel` directory,
by retrieving the list of files and their contents from a bare Git
repository.

```sh
# Obtain list of files in the bare repository
git --git-dir linux.git ls-tree -r --name-only v6.3 -- kernel |
# Use only C files
grep '\.[ch]$' |
# For each file
while read file; do
  echo "F$file"
  # Obtain and tokenize the file's contents
  git --git-dir linux.git show "v6.3:$file" |
  tokenizer -l C -o line -t N
done |
# Identify clones
mpcd -n 40
```


## Reference manual
You can read the command's Unix manual page through [this link](https://dspinellis.github.io/manview/?src=https%3A%2F%2Fraw.githubusercontent.com%2Fdspinellis%2Fmpcd%2Fmaster%2Fsrc%2Fmpcd.1&name=mpcd(1)&link=https%3A%2F%2Fgithub.com%2Fdspinellis%2mpcd).

