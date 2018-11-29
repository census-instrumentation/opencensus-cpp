# Specify structure for custom cmake functions
additional_commands = {
  "opencensus_lib": {
    "flags": [
      "PUBLIC",
    ],
    "kwargs": {
      "HEADERS": "*",
      "DEPENDS": "*",
      "SOURCES": "*"
    }
  }
}

# If comment markup is enabled, don't reflow the first comment block in
# eachlistfile. Use this to preserve formatting of your
# copyright/licensestatements.
first_comment_is_literal = True
