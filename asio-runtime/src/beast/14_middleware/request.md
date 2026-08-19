# Beast Router Middleware Examples

## Normal request

```bash
curl -i http://127.0.0.1:8080/
```

Example response headers:

```text
HTTP/1.1 200 OK
Content-Type: text/plain
X-Request-ID: req-1
X-Response-Time-us: 35
```

## Dynamic route

```bash
curl -i http://127.0.0.1:8080/users/42
```

## Protected route

Without authentication:

```bash
curl -i http://127.0.0.1:8080/private
```

Expected response:

```text
HTTP/1.1 401 Unauthorized
```

With authentication:

```bash
curl -i http://127.0.0.1:8080/private \
  -H "Authorization: Bearer secret"
```

Expected response:

```text
HTTP/1.1 200 OK
```