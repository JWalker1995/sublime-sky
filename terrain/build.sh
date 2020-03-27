#!/bin/sh

rm -rf SsProtocol

# Compile message schemas
flatc --python ../schemas/message.fbs