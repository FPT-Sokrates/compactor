# `compactor`

Data compression:
* with zero time for decompression
* with zero additional memory for decompression
* suited for read-only array data
* supports data alignment

## Quickstart Guide

1. Compile the program

    ```console
    cd src
    make 
    ```

2. Run compactor, e.g.

    ```console
    compactor -h
    ```

## Requirements

* c compiler (e.g. gcc)

## License

This project is licensed under the [MIT LICENSE](LICENSE.md).

Notice: Before you use the program in productive use, please take all necessary precautions, e.g. testing and verifying the program with regard to your specific use. The program was tested solely for our own use cases, which might differ from yours.
