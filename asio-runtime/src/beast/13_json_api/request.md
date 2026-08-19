````markdown
# User API Requests

## 1. Create a user

```bash
curl -X POST http://127.0.0.1:8080/users \
  -H "Content-Type: application/json" \
  -d '{"name":"Aftab"}'
```

## 2. Get that user

```bash
curl http://127.0.0.1:8080/users/1
```

### Response

```json
{
  "id": 1,
  "name": "Aftab"
}
```

## 3. Get all users

```bash
curl http://127.0.0.1:8080/users
```

### Response

```json
[
  {
    "id": 1,
    "name": "Aftab"
  }
]
```

## 4. Invalid JSON

```bash
curl -X POST http://127.0.0.1:8080/users \
  -H "Content-Type: application/json" \
  -d "hello"
```

### Response

```json
{
  "error": "Invalid JSON"
}
```
````