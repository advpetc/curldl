readme.txt

:Author: Haoyang
:Email: peter@peterchen.xyz
:Date: 2020-02-11 11:46

1. Download and Usage:
    1.1. I have provided a Makefile. To compile, simply type in: "make" in your command
            line. You can run make clean as well.
    1.2. There are three dependencies: g++, pthread and libcurl (which are default installed in
            most OS).
    1.3. For simplicity, all the download file are named with "file".
2. Design choice:
    2.1. In order to run the downloading process concurrently, I first create a
    	 function to retrieve the file size from the url. Then I can calculate the
    	 download range for each thread.
    2.2. I use a Node struct to associate with each thread to track the
    	 downloading process.
    2.3. I use pthread library to implement the multithreading part.
3. Performance:
        I used time command to test my downloader.

        Url for testing: https://a-hel-fi.m.voidlinux.org/live/current/void-armv6l-musl-ROOTFS-20191109.tar.xz

		Using 10 threads: ./downloader -c 10  0.40s user 0.33s system
        1% cpu 48.004 total

        Using 5 threads: ./downloader  -c 5  0.31s user 0.35s system 0% cpu
        1:45.69 total

        You can see the big time difference between these two (in regards to
        different threads number).
