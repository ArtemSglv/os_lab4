# Use an official Python runtime as a parent image
FROM ubuntu

# Set the working directory to /app
WORKDIR /lab4

# Copy the current directory contents into the container at /app
ADD . /lab4
ADD . /lab4/zin
RUN apt-get update
RUN apt-get --force-yes -y install gcc
RUN gcc -pthread -o l4 lab4.c


# Define environment variable
#ENV NAME World

# Run app.py when the container launches
CMD "./l4"