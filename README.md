# Vertica Extension Functions

![GitHub](https://img.shields.io/github/license/h-serizawa/vertica-extension-functions)

This repository contains the User-Defined Extension (UDx) codes to extend the processing capabilities of the Vertica Analytics Platform.

You need the Vertica SDK to compile these codes and you need the Vertica Analytics Platform to run them.

All codes are provided under the license found in LICENSE file.

## Current functions

- Long2Wide function

    Transforms Long-form data to Wide-form data like PIVOT feature.

- ImplodeExt function

    Assembles data into array grouped by keys. It can support ARRAY and ROW complex type.

- MallocInfo function

    Logs the output of malloc_info() system call on initiator node.

- KafkaRemoveMagicByte filter

    Removes the unexpected data in front of the JSON data consumed from Apache Kafka.

Enjoy your life with Vertica!!
