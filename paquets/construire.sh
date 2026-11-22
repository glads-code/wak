#!/bin/sh
# Construit le paquet de wak dans un conteneur jetable (lancé par
# paquets.rc) : .deb sous Debian, .rpm sous Fedora. Source en lecture
# seule dans /src, paquet écrit dans /dist.
# Usage : construire.sh deb|rpm <version>
set -eu
type=$1 version=$2
cp -r /src /tmp/wak && cd /tmp/wak
case $type in
deb)
	apt-get update -qq && apt-get install -y -qq gcc libc6-dev 9base >/dev/null
	gcc -O2 -std=c99 -D_POSIX_C_SOURCE=200809L -o wak wak.c -lm
	PATH=/usr/lib/plan9/bin:$PATH ./essai.rc ./wak
	p=/tmp/p/wak_${version}-1_amd64
	install -Dm755 wak $p/usr/bin/wak
	install -Dm644 LICENSE $p/usr/share/doc/wak/copyright
	install -Dm644 README.md $p/usr/share/doc/wak/README.md
	mkdir -p $p/DEBIAN
	cat > $p/DEBIAN/control <<FIN
Package: wak
Version: ${version}-1
Architecture: amd64
Maintainer: GLADS <contact@glads.fr>
Depends: libc6 (>= 2.34)
Section: utils
Priority: optional
Homepage: https://github.com/glads-code/wak
Description: awk compatible UTF-8 (wak de Ray Gardner, awk de toybox)
 Petit awk qui compte en caractères UTF-8 quelle que soit la locale,
 écrit toujours les nombres avec un point et accepte les variables
 d'environnement en double. Publié sans support.
FIN
	dpkg-deb --root-owner-group --build $p /dist/ >/dev/null
	;;
rpm)
	dnf -q -y install gcc glibc-devel rpm-build >/dev/null
	mkdir -p ~/rpmbuild/SOURCES
	tar -C /tmp -czf ~/rpmbuild/SOURCES/wak-$version.tar.gz --transform "s,^wak,wak-$version," wak
	rpmbuild -bb --quiet --define "version $version" paquets/wak.spec
	cp ~/rpmbuild/RPMS/x86_64/wak-$version-*.rpm /dist/
	;;
esac
