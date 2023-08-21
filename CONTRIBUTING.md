# Contributing to Inamata Controller Firmware

:+1::tada: First off, thanks for taking the time to contribute! :tada::+1:

The following is a set of guidelines for contributing to the Inamata Controller Firmware. The repository is hosted in a private GitLab repository and synced with this public GitHub repository. These are mostly guidelines, not rules. Use your best judgment, and feel free to propose changes to this document in a pull request.

## How Can I Contribute

Beside reporting bugs and suggesting enhancements, we are always open to supporting new hardware in the mainline firmware.

### General Support

The best place for general support is the forum (to be created). This is especially true if you're looking to ask about approaches regarding the entire Inamata platform.

### Reporting Bugs

To report bugs, create an issue with the [bug report template][2].

### Suggesting Enhancements

To request features, create an issue with the [feature request template][3].

### Your First Code Contribution

The firmware can be flashed directly to the ESP32 (or ESP8285 with 2MB flash) from VS Code with the PlatformIO extension. This allows you to quickly iterate on code changes. As connection settings are saved to a separate partition, it is possible to reuse them without configuring them after flashing a new firmware. Additional information can be found in the _doc_ folder.

- [Auth Registration][4]
- [Flash and Boot][5]
- [Peripherals][6]
- [WebSocket API][7]

### Pull Requests

The process of upstreaming pull request changes follows the steps below.

1. Open a pull request
2. A team member will review it
3. Iterate changes until it is acceptable
4. A team member will upsteam the branch to our internal GitLab repository and publish changes
5. New commit is automatically downsteamed to this GitHub repository
6. The new firmware is made available on the platform

## Styleguides

### Git Commit Messages

Commits are squashed and rebased. A commit message using the following template is created:

```
Some awesome title

Why:

- ...

This change addresses the need by:

- ...
```

Include the above template in your merge request description. Once the merge request has passed review, we will push the changes to our upstream GitLab repository and commit the changes. This will include you as author of the commit.

### C++ Styleguide

The code is formatted with clang-format with the [Google C++ style][1].

### Documentation Styleguide

Please use English, avoid profanity and ensure legibility :smile:

## Additional Notes

[1]: https://google.github.io/styleguide/cppguide.html
[2]: https://github.com/InamataCo/Controller/issues/new?template=bug_report.md
[3]: https://github.com/InamataCo/Controller/issues/new?template=feature_request.md
[4]: doc/auth_registration.md
[5]: doc/flash_and_boot.md
[6]: doc/peripherals.md
[7]: doc/websocket_api.md
