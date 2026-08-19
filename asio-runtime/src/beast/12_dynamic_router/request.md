# Dynamic Beast Router Examples

## Static route

```bash
curl http://127.0.0.1:8080/
```

## Path parameter

```bash
curl http://127.0.0.1:8080/users/42
```

User ID = `42`

### Try another

```bash
curl http://127.0.0.1:8080/users/987
```

User ID = `987`

The router dynamically extracts the user ID:

```text
/users/987
       │
       ▼
params["id"] = "987"
```

## Multiple path segments

```bash
curl http://127.0.0.1:8080/users/42/profile
```

Profile of user `42`

## Query parameter

```bash
curl "http://127.0.0.1:8080/search?q=beast"
```

Search query = `beast`

## Multiple query parameters

```bash
curl "http://127.0.0.1:8080/search?q=beast&sort=name"
```

Conceptually:

```text
path
    /search

query
    q    = beast
    sort = name
```