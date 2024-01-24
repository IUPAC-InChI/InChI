FROM python:3.11

# Python image includes the dependencies we need for building InChI (gcc, make, etc.)

RUN pip install --upgrade pip && \
    pip install git+https://github.com/TUCAN-nest/SDF-pipeline.git
