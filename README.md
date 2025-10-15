# oOPS

Accidentally typed something confidential into the terminal?

![Meme](https://i.imgflip.com/a95uxa.jpg)

## Usage

In a scenario where you type something you didn't wanna save in the shell history.

Just removing the last line

```bash
> export TOKEN="__REDACTED__"
> oops
```

Removing multiple lines

```bash
> export TOKEN="I"
> export TOKEN="keep"
> export TOKEN="leaking"
> export TOKEN="SECRETS"
> oops 4
```

## Installation

Clone this repo:

```bash
git clone https://github.com/mdhvg/oops
cd oops
```

Install

```bash
make install
```

Installing in a custom directory

```bash
make install PREFIX="/path/to/custom/dir"
```

## Uninstall

Just run

```bash
make uninstall
```

Or in case of custom path

```bash
make uninstall PREFIX="/path/to/install/dir"
```

### Currently supported shells
- Zsh

> Feel free to make a PR for your preferred terminal :)