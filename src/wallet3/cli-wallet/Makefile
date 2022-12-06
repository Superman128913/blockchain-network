PYTHON_MAJOR_VERSION=3
PYTHON_MINOR_VERSION=8
PYTHON_VERSION=$(PYTHON_MAJOR_VERSION).$(PYTHON_MINOR_VERSION)
PYTHON_WITH_VERSION=python$(PYTHON_VERSION)
PIP_WITH_VERSION=pip$(PYTHON_VERSION)

all: build

system_dependencies:
	$(PIP_WITH_VERSION) install --upgrade setuptools
	$(PIP_WITH_VERSION) install --upgrade build

build:
	$(PYTHON_WITH_VERSION) -m build
	$(PIP_WITH_VERSION) install --editable .

run:
	oxen_wallet_cli

clean:
	find . -type d -name "__pycache__" -exec rm -r {} +
	find . -type d -name "*.egg-info" -exec rm -r {} +
	rm -rf dist/ build/
