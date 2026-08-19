# Beast Router Examples

## Root route

```bash
curl http://127.0.0.1:8080/
```

Response:

```text
Hello from Beast Router!
```

## Hello route

```bash
curl http://127.0.0.1:8080/hello
```

Response:

```text
Hello!
```

## Thread route

```bash
curl http://127.0.0.1:8080/thread
```

Response:

```text
Handled by thread: 123456789
```

## Echo route

```bash
curl -X POST http://127.0.0.1:8080/echo -d "hello beast"
```

Response:

```text
hello beast
```