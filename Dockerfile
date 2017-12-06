# Use an official Python runtime as a parent image
FROM ubuntu

# Set the working directory to /app
WORKDIR /lab4

# Copy the current directory contents into the container at /app
ADD . /lab4
RUN apt-get update
RUN apt-get --force-yes -y install gcc


# Define environment variable
#ENV NAME World

# Run app.py when the container launches
CMD "/bin/bash"