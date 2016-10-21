#!/bin/bash

set -e

PROJECT_OWNER=zmarcantel
PROJECT_NAME=cpplatex
SSH_KEY_TRAVIS_ID=458ec19346cc

DOCSDIR=docs


if [[ "$TRAVIS_BRANCH" != master || "$TRAVIS_PULL_REQUEST" = true ]]; then
    echo "refusing to build docs for branch or PR"
    exit 0
fi

eval key=\$encrypted_${SSH_KEY_TRAVIS_ID}_key
eval iv=\$encrypted_${SSH_KEY_TRAVIS_ID}_iv

mkdir -p ~/.ssh
openssl aes-256-cbc -K $key -iv $iv -in ci/travis-cpplatex.enc -out ~/.ssh/id_rsa -d
chmod 600 ~/.ssh/id_rsa

# clone the gh-pages branch --> ./docs
git clone --branch gh-pages git@github.com:$PROJECT_OWNER/$PROJECT_NAME $DOCSDIR

# set up the repo config
rm -rf $DOCSDIR/*
cd $DOCSDIR
echo "" > .nojekyll
git config user.name "Travis-Generated Documentation"
git config user.email "$PROJECT_NAME@travis-ci.org"
cd ..

echo "generating docs"
doxygen

# create an index.html redirect
echo "creating index.html"
echo "<meta http-equiv=\"refresh\" content=\"0; URL='html/index.html'\" />" > $DOCSDIR/index.html


# go into the "new" repo with the docs
cd $DOCSDIR

git add --all .
echo "commiting...."
git commit -m "doc upload for $PROJECT_NAME ($TRAVIS_COMMIT)"
echo "pushing..."
git push origin gh-pages
