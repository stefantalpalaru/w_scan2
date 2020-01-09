#
# This script will be called by uscan and will remove the precompiled
# binary from the upstream tarball
#

# Parse parameters passed by uscan;
while [ $# -ge 1 ]; do
    case $1 in
      --upstream-version)
          shift
          UPSTREAM_VERSION="$1"
          ;;
      --)
          shift
          ;;
      *)
          TARBALL="$1"
          ;;
    esac
    shift
done

echo "Extracting tarball..."
tar -xjf "$TARBALL"

echo "Stripping precompiled binary..."
rm w_scan-$UPSTREAM_VERSION/w_scan

echo "Fixing file modes..."
chmod a-x w_scan-$UPSTREAM_VERSION/doc/*

echo "Repacking tarball..."
tar -czf "`dirname $TARBALL`/w-scan_$UPSTREAM_VERSION.orig.tar.gz" "w_scan-$UPSTREAM_VERSION"

echo "Cleanup..."
rm -rf w_scan-$UPSTREAM_VERSION
