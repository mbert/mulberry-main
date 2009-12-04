%define mulberry_version      4.0.9b1
%define BuiltDir             /users/dev/Mulberry/Build
%define TarDir               /users/dev/Mulberry/Build/obj
%define _topdir              %BuiltDir/rpmdir
%define _rpmfilename         %%{NAME}-%%{VERSION}.%%{ARCH}.rpm

Summary: Mulberry is an Internet Email client supporting IMAP & POP3.
Name: mulberry
Version: %mulberry_version
Release: 0
ExclusiveArch: i386
ExclusiveOS: Linux
Copyright: Apache 2 License.
Group: Applications/Internet
Source0: mulberry.tar
AutoReqProv: no
BuildRoot: %_topdir

%description
                   Mulberry
       Internet Mail from the Ground Up

Mulberry is a commercial GUI-based email client
supporting the IMAP and POP3 internet protocols for
remote storage and manipulation of email, or local
mailboxes. It also supports SMTP for sending email,
as well as IMSP, ACAP and LDAP internet protocols
for remote preferences, remote address books and
directory services.

Mulberry is currently available for Macintosh, Win32 and
various Unix flavors, including Linux and Solaris.

<http://www.mulberrymail.com>

%prep
cd $RPM_BUILD_ROOT/SOURCES
rm mulberry.tar
mkdir mulberry-%mulberry_version
tar cf mulberry.tar mulberry-%mulberry_version
cd ../..

rm -rf $RPM_BUILD_ROOT/BUILD
mkdir $RPM_BUILD_ROOT/BUILD

rm -rf $RPM_BUILD_ROOT/RPMS
mkdir $RPM_BUILD_ROOT/RPMS

rm -rf $RPM_BUILD_ROOT/usr
mkdir $RPM_BUILD_ROOT/usr
mkdir $RPM_BUILD_ROOT/usr/local
mkdir $RPM_BUILD_ROOT/usr/local/bin
mkdir $RPM_BUILD_ROOT/usr/local/lib
mkdir $RPM_BUILD_ROOT/usr/local/lib/mulberry
mkdir $RPM_BUILD_ROOT/usr/local/share
mkdir $RPM_BUILD_ROOT/usr/local/share/doc
mkdir $RPM_BUILD_ROOT/usr/local/share/doc/mulberry
mkdir $RPM_BUILD_ROOT/usr/local/share/mulberry

%setup 

%install
cp %TarDir/mulberry $RPM_BUILD_ROOT/usr/local/bin
cp -r %TarDir/.mulberry/Plug-ins $RPM_BUILD_ROOT/usr/local/lib/mulberry
cp %TarDir/.mulberry/CHANGES* $RPM_BUILD_ROOT/usr/local/share/doc/mulberry
cp %TarDir/Mulberry_License $RPM_BUILD_ROOT/usr/local/share/doc/mulberry
cp -r %TarDir/.mulberry/icons $RPM_BUILD_ROOT/usr/local/share/mulberry

%post

%postun

%files

/usr/local/bin/mulberry
/usr/local/lib/mulberry
/usr/local/share/mulberry
/usr/local/share/doc/mulberry


