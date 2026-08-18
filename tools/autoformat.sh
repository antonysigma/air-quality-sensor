#!/bin/bash

ruff format ./ &&\
    ruff check --fix --select I,E ./
