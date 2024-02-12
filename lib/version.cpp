//
// Created by Mateusz on 08.02.2024.
//

#include "version.hpp"

#include "remote_connector.hpp"

inseye::Version::Version(const inseye::Version &other) noexcept : major(other.major), minor(other.minor), patch(other.patch) {}

inseye::Version::Version(const Version &&other) noexcept : major(other.major), minor(other.minor), patch(other.patch) {}

inseye::Version& inseye::Version::operator=(Version && moved)  noexcept {
    this->major = moved.major;
    this->minor = moved.minor;
    this->patch = moved.patch;
    return *this;
}

bool inseye::Version::operator==(const inseye::Version &other) const {
    return !(*this != other);
}


bool inseye::Version::operator!=(const inseye::Version &other) const {
    if (this->major != other.major)
        return false;
    if (this->minor != other.minor)
        return false;
    if (this->patch != other.patch)
        return false;
    return true;
}

bool inseye::Version::operator<(const inseye::Version &other) const {
    if (major < other.major)
        return true;
    if (major > other.major)
        return false;
    if (minor < other.minor)
        return true;
    if (minor > other.minor)
        return false;
    return patch < other.patch; 
}

bool inseye::Version::operator>(const inseye::Version &other) const {
    return other < *this;
}

bool inseye::Version::operator>=(const inseye::Version &other) const {
    return !(*this < other);
}

bool inseye::Version::operator<=(const inseye::Version &other) const {
    return !(*this > other);
}

uint32_t inseye::Version::GetMajor() const noexcept {
    return major;
}

uint32_t inseye::Version::GetMinor() const noexcept {
    return minor;
}

uint32_t inseye::Version::GetPatch() const noexcept {
    return patch;
}




