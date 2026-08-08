#!/usr/bin/env bash
#
# Package the Qt GUI as a self-contained, self-signed macOS .app bundle.
#
# Runs macdeployqt to copy the Qt frameworks and plugins into the bundle,
# then code signs everything with an ad-hoc signature.
#
# Ad-hoc signing (`codesign -s -`) is what you can produce without a $99/yr
# Apple Developer Program membership. It is enough to:
#   - satisfy the hard requirement on Apple Silicon that all code be signed
#   - keep the bundle's own integrity check valid
#   - let the app run normally once macOS has been told to trust it
#
# It is NOT notarization. A bundle downloaded from the internet still gets
# the com.apple.quarantine attribute, and Gatekeeper will refuse it with
# "app is damaged" until the user strips it:
#
#     xattr -dr com.apple.quarantine /Applications/super-snes9x-qt.app
#
# See qt/docs/README-macos.md for the full story.
#
# Usage:
#   ./makeapp-macos.sh [build-dir]      # default: qt/build-macos
#   SIGN_IDENTITY="Developer ID Application: ..." ./makeapp-macos.sh
#   MAKE_DMG=1 ./makeapp-macos.sh
#
# Requirements: a completed cmake build, plus macdeployqt from the same Qt.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${1:-${QT_DIR}/build-macos}"
APP_NAME="super-snes9x-qt.app"
APP="${BUILD_DIR}/${APP_NAME}"

# "-" means an ad-hoc signature. Override to use a real certificate if you
# ever get one; the rest of the script is identical either way.
SIGN_IDENTITY="${SIGN_IDENTITY:--}"
MAKE_DMG="${MAKE_DMG:-0}"

step() { echo; echo "==> $*"; }

find_macdeployqt() {
    if command -v macdeployqt >/dev/null 2>&1; then
        command -v macdeployqt
        return
    fi
    # Homebrew keeps it out of PATH by default (keg-only qt).
    for prefix in "$(brew --prefix qt 2>/dev/null || true)" \
                  /usr/local/opt/qt /opt/homebrew/opt/qt; do
        if [ -n "${prefix}" ] && [ -x "${prefix}/bin/macdeployqt" ]; then
            echo "${prefix}/bin/macdeployqt"
            return
        fi
    done
    return 1
}

if [ ! -d "${APP}" ]; then
    echo "error: ${APP} not found -- build it first, e.g." >&2
    echo "  cmake -S qt -B ${BUILD_DIR} -G Ninja -DCMAKE_BUILD_TYPE=Release \\" >&2
    echo "        -DCMAKE_PREFIX_PATH=\"\$(brew --prefix qt)\"" >&2
    echo "  cmake --build ${BUILD_DIR} -j\"\$(sysctl -n hw.ncpu)\"" >&2
    exit 1
fi

MACDEPLOYQT="$(find_macdeployqt)" || {
    echo "error: macdeployqt not found (install Qt 6, e.g. brew install qt)" >&2
    exit 1
}

# CMake emits Contents/Info.plist at generate time rather than as a build
# rule, so deleting the .app and running only `cmake --build` produces a
# bundle without one. That still launches, which is why it is worth failing
# on here: the app would have no bundle identifier, no icon and no
# NSHighResolutionCapable, and macOS would fall back to the executable name.
if [ ! -f "${APP}/Contents/Info.plist" ]; then
    echo "error: ${APP_NAME} has no Contents/Info.plist" >&2
    echo "       re-run cmake to regenerate it:" >&2
    echo "         cmake -S \"$(dirname "${QT_DIR}")/qt\" -B \"${BUILD_DIR}\"" >&2
    exit 1
fi

step "Deploying Qt frameworks into ${APP_NAME}"
# Homebrew's qt is a meta-formula: the frameworks the app links against
# resolve through the qtbase keg, but modules like QtSvg and QtPdf live in
# kegs of their own, and none of those directories appear in the binary's
# rpath list. macdeployqt then cannot resolve them, skips them, and -- since
# it still exits 0 -- would leave a bundle whose iconengines/libqsvgicon
# plugin has no QtSvg to load, so every SVG icon silently fails to render.
# <macdeployqt>/../lib is the aggregated prefix holding every framework.
QT_LIB_DIR="$(cd "$(dirname "${MACDEPLOYQT}")/../lib" 2>/dev/null && pwd || true)"
if [ -n "${QT_LIB_DIR}" ]; then
    echo "    searching ${QT_LIB_DIR} for Qt modules"
    DEPLOY_LIBPATH="-libpath=${QT_LIB_DIR}"
else
    DEPLOY_LIBPATH=""
fi

# -always-overwrite keeps repeated runs from mixing an old Qt into the bundle.
DEPLOY_LOG="$(mktemp -t macdeployqt)"
if [ -n "${DEPLOY_LIBPATH}" ]; then
    "${MACDEPLOYQT}" "${APP}" -always-overwrite "${DEPLOY_LIBPATH}" 2>&1 | tee "${DEPLOY_LOG}"
else
    "${MACDEPLOYQT}" "${APP}" -always-overwrite 2>&1 | tee "${DEPLOY_LOG}"
fi

# macdeployqt reports unresolved dependencies and still exits 0. These are
# advisory here rather than fatal: the completion pass below fixes the ones
# that matter, and the self-contained check after it is the real gate. Note
# them so an unexpected one is still visible.
if grep -q "^ERROR: Cannot resolve" "${DEPLOY_LOG}"; then
    echo "    macdeployqt could not resolve the following (handled below):"
    grep "^ERROR: Cannot resolve" "${DEPLOY_LOG}" \
        | sed 's/^ERROR: Cannot resolve rpath /      /' | sort -u
fi
rm -f "${DEPLOY_LOG}"

step "Pruning plugins this app does not use"
# Each of these drags in a framework of its own (QtPdf, QtVirtualKeyboard)
# or extra image codecs. None are reachable from a desktop SNES emulator, and
# dropping them removes most of what macdeployqt could not resolve.
for plugin in \
    "PlugIns/imageformats/libqpdf.dylib" \
    "PlugIns/platforminputcontexts/libqtvirtualkeyboardplugin.dylib" \
    "PlugIns/imageformats/libqwebp.dylib"; do
    if [ -e "${APP}/Contents/${plugin}" ]; then
        echo "    removing ${plugin}"
        rm -f "${APP}/Contents/${plugin}"
    fi
done

step "Completing the bundle"
# macdeployqt resolves a framework using only the rpaths of the binary it is
# scanning, and -libpath does not extend that search. Homebrew's Qt plugins
# carry a single relative rpath (@loader_path/../../../../lib) which stops
# meaning anything once the plugin is copied into the bundle, so modules kept
# in sibling kegs -- QtSvg above all, which the qsvgicon engine needs to draw
# every toolbar icon -- are silently skipped. Copy in whatever is still
# missing and repoint it, looping until the dependency graph closes.
FRAMEWORKS_DIR="${APP}/Contents/Frameworks"
mkdir -p "${FRAMEWORKS_DIR}"

SEARCH_DIRS="${QT_LIB_DIR:-} /usr/local/lib /usr/local/opt/qt/lib /opt/homebrew/lib"

# Every Mach-O in the bundle, NUL-separated. Selecting on the file's actual
# type rather than on "executable bit or .dylib suffix" matters: a framework's
# binary (Frameworks/QtSvg.framework/Versions/A/QtSvg) has neither, so a
# name-based sweep silently skips exactly the files most likely to still
# point at Homebrew.
bundle_machos() {
    find "${APP}/Contents" -type f -print0 | while IFS= read -r -d '' f; do
        case "$(file -b "$f" 2>/dev/null)" in
            *Mach-O*) printf '%s\0' "$f" ;;
        esac
    done
}

# Every @rpath dependency in the bundle that is not satisfied inside it.
list_missing() {
    while IFS= read -r -d '' macho; do
        otool -L "${macho}" 2>/dev/null | awk '/@rpath\//{print $1}'
    done < <(bundle_machos) \
    | sed 's|^@rpath/||' | sort -u | while IFS= read -r ref; do
        case "${ref}" in
            *.framework/*)
                fw="${ref%%.framework/*}.framework"
                [ -d "${FRAMEWORKS_DIR}/${fw}" ] || echo "${fw}"
                ;;
            *.dylib)
                lib="${ref##*/}"
                [ -f "${FRAMEWORKS_DIR}/${lib}" ] || echo "${lib}"
                ;;
        esac
    done | sort -u
}

locate_item() {
    for dir in ${SEARCH_DIRS}; do
        [ -n "${dir}" ] || continue
        candidate="${dir}/$1"
        [ -e "${candidate}" ] || continue
        if [ -d "${candidate}" ]; then
            # Homebrew links frameworks into /usr/local/lib as symlinks with
            # relative targets, which dangle once copied into the bundle.
            # Resolve to the real directory so cp -R copies the framework
            # itself -- while still preserving the symlinks *inside* it
            # (Versions/Current), which codesign requires.
            (cd "${candidate}" && pwd -P)
        else
            real_dir="$(cd "$(dirname "${candidate}")" && pwd -P)"
            echo "${real_dir}/$(basename "${candidate}")"
        fi
        return 0
    done
    return 1
}

for _pass in 1 2 3 4 5; do
    missing="$(list_missing)"
    [ -n "${missing}" ] || break

    progressed=0
    while IFS= read -r item; do
        [ -n "${item}" ] || continue
        if src="$(locate_item "${item}")"; then
            echo "    adding ${item}"
            if [ -d "${src}" ]; then
                cp -R "${src}" "${FRAMEWORKS_DIR}/"
            else
                # -L: a plain dylib in /usr/local/lib is usually a symlink
                # into the Cellar, and the link would dangle in the bundle.
                cp -L "${src}" "${FRAMEWORKS_DIR}/${item}"
            fi
            chmod -R u+w "${FRAMEWORKS_DIR}/${item}"
            case "${item}" in
                *.framework)
                    name="${item%.framework}"
                    binary="${FRAMEWORKS_DIR}/${item}/Versions/A/${name}"
                    [ -f "${binary}" ] || binary="${FRAMEWORKS_DIR}/${item}/${name}"
                    # Headers and .prl files are build-time only and upset
                    # codesign's bundle validation.
                    rm -rf "${FRAMEWORKS_DIR}/${item}/Headers" \
                           "${FRAMEWORKS_DIR}/${item}/Versions/A/Headers" \
                           "${FRAMEWORKS_DIR}/${item}"/*.prl
                    install_name_tool -id \
                        "@rpath/${item}/Versions/A/${name}" "${binary}" 2>/dev/null || true
                    ;;
                *)
                    install_name_tool -id \
                        "@rpath/${item}" "${FRAMEWORKS_DIR}/${item}" 2>/dev/null || true
                    ;;
            esac
            progressed=1
        else
            echo "    warning: cannot locate ${item}" >&2
        fi
    done <<EOF
${missing}
EOF

    [ "${progressed}" -eq 1 ] || break
done

step "Repointing absolute dependencies at the bundle"
# macdeployqt rewrites install names only for what it deployed itself, so
# anything copied in above still refers to Homebrew by absolute path -- e.g.
# QtSvg links /usr/local/opt/qtbase/lib/QtCore.framework. dyld then loads a
# second copy of QtCore alongside the bundled one, which means duplicate objc
# classes and, because each QtCore keeps its own resource registry, every
# ":/icons/..." lookup failing. Point them back inside the bundle.
while IFS= read -r -d '' macho; do
    while IFS= read -r dep; do
        case "${dep}" in
            /usr/local/*|/opt/homebrew/*) ;;
            *) continue ;;
        esac
        case "${dep}" in
            *.framework/*)
                fw="${dep##*/}"
                fw="${fw}.framework"
                rel="${dep#*"${fw}"/}"
                if [ -d "${FRAMEWORKS_DIR}/${fw}" ]; then
                    install_name_tool -change "${dep}" \
                        "@rpath/${fw}/${rel}" "${macho}" 2>/dev/null || true
                fi
                ;;
            *.dylib)
                lib="${dep##*/}"
                # Skip the library's own id, which legitimately names itself.
                if [ -f "${FRAMEWORKS_DIR}/${lib}" ] && \
                   [ "${macho##*/}" != "${lib}" ]; then
                    install_name_tool -change "${dep}" \
                        "@rpath/${lib}" "${macho}" 2>/dev/null || true
                fi
                ;;
        esac
    done < <(otool -L "${macho}" 2>/dev/null | awk 'NR>1{print $1}')
done < <(bundle_machos)

step "Checking the bundle is self-contained"
# Belt and braces: whatever the reason, every @rpath framework referenced by
# anything in the bundle must actually be inside it. A missing one loads fine
# on this machine (the system Qt is still on disk) and fails on any other, so
# it has to be caught here rather than by a user.
unresolved=0
while IFS= read -r -d '' macho; do
    # Any @rpath framework must actually be inside the bundle.
    while IFS= read -r dep; do
        fw="${dep#@rpath/}"
        fw="${fw%%/*}"
        case "${fw}" in
            *.framework)
                if [ ! -d "${FRAMEWORKS_DIR}/${fw}" ]; then
                    echo "  missing ${fw} (needed by ${macho#"${APP}/"})" >&2
                    unresolved=1
                fi
                ;;
        esac
    done < <(otool -L "${macho}" 2>/dev/null | awk '/@rpath\/.*\.framework/{print $1}')

    # And nothing may still link Homebrew by absolute path: it would load a
    # second Qt on this machine and simply be absent on any other.
    while IFS= read -r dep; do
        case "${dep}" in
            /usr/local/*|/opt/homebrew/*)
                [ "${dep##*/}" = "${macho##*/}" ] && continue  # its own id
                echo "  absolute dep ${dep} (in ${macho#"${APP}/"})" >&2
                unresolved=1
                ;;
        esac
    done < <(otool -L "${macho}" 2>/dev/null | awk 'NR>1{print $1}')
done < <(bundle_machos)

if [ "${unresolved}" -ne 0 ]; then
    echo >&2
    echo "error: bundle is not self-contained" >&2
    exit 1
fi
echo "    all dependencies resolve inside the bundle"

step "Stripping external rpaths"
# macdeployqt copies Qt in and rewrites the install names, but leaves the
# original LC_RPATH (e.g. /usr/local/opt/qt/lib) in place. dyld then loads
# BOTH the bundled and the system Qt: the objc runtime warns about duplicate
# classes, and -- worse -- each QtCore gets its own resource registry, so the
# qrc icons registered in one are invisible to the other and every
# ":/icons/..." lookup fails. Drop any rpath that points outside the bundle.
# Applied to every Mach-O in the bundle, not just the main executable: the
# frameworks copied in above carry their own absolute rpaths too.
while IFS= read -r -d '' macho; do
    while IFS= read -r rpath; do
        case "${rpath}" in
            @*) ;;  # @executable_path/... etc. stay
            *)  echo "    removing ${rpath} from ${macho#"${APP}/"}"
                install_name_tool -delete_rpath "${rpath}" "${macho}" 2>/dev/null || true ;;
        esac
    done < <(otool -l "${macho}" 2>/dev/null | awk '/LC_RPATH/{f=1} f&&/path /{print $2; f=0}')
done < <(bundle_machos)

step "Signing (identity: ${SIGN_IDENTITY})"
# Sign inside-out. `codesign --deep` is deprecated and signs nested code with
# the *outer* options, which silently produces bundles that fail --strict
# verification, so walk the nested Mach-O files explicitly instead.
while IFS= read -r -d '' item; do
    codesign --force --sign "${SIGN_IDENTITY}" --timestamp=none "${item}"
done < <(find "${APP}/Contents" \
              \( -name "*.dylib" -o -name "*.so" \) -type f -print0)

# Frameworks are signed as bundles, not as bare Mach-O files.
if [ -d "${APP}/Contents/Frameworks" ]; then
    while IFS= read -r -d '' framework; do
        codesign --force --sign "${SIGN_IDENTITY}" --timestamp=none "${framework}"
    done < <(find "${APP}/Contents/Frameworks" -name "*.framework" -maxdepth 1 -print0)
fi

# Finally the bundle itself, which seals everything above.
codesign --force --sign "${SIGN_IDENTITY}" --timestamp=none "${APP}"

step "Verifying signature"
codesign --verify --deep --strict --verbose=2 "${APP}"

# Gatekeeper assessment is expected to FAIL for an ad-hoc signature -- it is
# reported here for transparency, not treated as an error.
echo
echo "Gatekeeper assessment (an ad-hoc signature is expected to be rejected):"
spctl --assess --type execute --verbose=4 "${APP}" 2>&1 || true

if [ "${MAKE_DMG}" -eq 1 ]; then
    step "Building DMG"
    DMG="${BUILD_DIR}/super-snes9x-qt.dmg"
    rm -f "${DMG}"
    hdiutil create -volname "Super Snes9x" -srcfolder "${APP}" \
        -ov -format UDZO "${DMG}" >/dev/null
    codesign --force --sign "${SIGN_IDENTITY}" --timestamp=none "${DMG}"
    echo "    ${DMG}"
fi

echo
echo "================ Build artifacts ================"
echo "  ${APP}"
[ "${MAKE_DMG}" -eq 1 ] && echo "  ${BUILD_DIR}/super-snes9x-qt.dmg"
echo "================================================="
echo
echo "The bundle is self-signed, not notarized. On another Mac, strip the"
echo "quarantine attribute after copying it over:"
echo "  xattr -dr com.apple.quarantine /Applications/${APP_NAME}"
