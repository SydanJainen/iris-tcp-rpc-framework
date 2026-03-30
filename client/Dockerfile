FROM python:3.11-slim

WORKDIR /app

COPY client/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY client/src/ src/

ENTRYPOINT ["python", "-m", "src.main"]
