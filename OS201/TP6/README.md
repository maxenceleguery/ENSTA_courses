# OS201 course - TP6

Basic commands parser

## Compilation

```bash
make
```

## Execution

```bash
echo "+3,2" | output/main
```

-> 5

```bash
echo "-8,2" | output/main
```

-> 6

```bash
echo "els ~" | output/main
```

## Attack

Use of the vulnerability to steal information. Blocked by the encapsulation of the process.

```bash
cat attack | output/main 
```
