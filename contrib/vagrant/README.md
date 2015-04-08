# Vagrant

Build the latest version of the MeTA toolkit using [Vagrant](https://www.vagrantup.com/). Ubuntu 14.04 is used as a base Vagrant box. This project was initially made for [Text Retrieval and Search Engines](https://www.coursera.org/course/textretrieval) Coursera course.

## Instructions

1. Install Vagrant  
2. Run Vagrant
    ```
    vagrant up
    ```
    
3. Check that everything is OK
  ```
    vagrant ssh
    cd meta/build
    ctest --output-on-failure
    ```

## Notes

Only VirtualBox provider was tested. 
