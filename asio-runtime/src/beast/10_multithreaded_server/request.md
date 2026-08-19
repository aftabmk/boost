# Beast Router Examples

## Root route

```bash
curl http://127.0.0.1:8080/
```

Response:

```text
Hello from multi-threaded Beast!
```

## Get thread ID (TID)

```bash
curl http://127.0.0.1:8080/thread
```

Response:

```text
Request processed by worker thread: 8670141377090704656
```