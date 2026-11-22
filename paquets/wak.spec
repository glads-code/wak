Name:           wak
Version:        %{version}
Release:        1%{?dist}
Summary:        awk compatible UTF-8 (wak de Ray Gardner, awk de toybox)
License:        0BSD
URL:            https://github.com/glads-code/wak
Source0:        wak-%{version}.tar.gz
BuildRequires:  gcc

%description
Petit awk qui compte en caractères UTF-8 quelle que soit la locale,
écrit toujours les nombres avec un point et accepte les variables
d'environnement en double. Publié sans support.

%prep
%autosetup

%build
gcc %{optflags} -std=c99 -D_POSIX_C_SOURCE=200809L -o wak wak.c -lm

%check
# Mêmes points que essai.rc (qui demande le rc de Plan 9, absent de Fedora)
test "$(echo église | ./wak '{ print length($0), substr($0, 1, 2), toupper($0) }')" = "6 ég ÉGLISE"
test "$(./wak 'BEGIN { print 7 / 2, sprintf("%.0f", 2 ^ 40) }')" = "3.5 1099511627776"
test "$(LC_ALL=fr_FR.UTF-8 ./wak 'BEGIN { printf "%s-%02d|%.1f\n", 2026, 8, 7 / 2 }')" = "2026-08|3.5"
test "$(printf 'a\nb\n' | ./wak '{ print; nextfile }')" = "a"
test "$(env 'X=1' 'X=2' ./wak 'BEGIN { print "ok" }')" = "ok"

%install
install -Dm755 wak %{buildroot}%{_bindir}/wak

%files
%license LICENSE
%doc README.md
%{_bindir}/wak

%changelog
* Sun Nov 22 2026 GLADS <contact@glads.fr> - 1.0-1
- Première version empaquetée
