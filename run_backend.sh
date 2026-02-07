docker run \
  -p 8080:8080 \
  --add-host=host.docker.internal:host-gateway \
  -v $(pwd)/system/config.json:/app/build/config.json \
  -v $(pwd)/static:/app/build/static \
  infusion-backend:v1
