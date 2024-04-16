# Usage

Terminal only works as a manual keyboard operated prorocol if the terninal sends CR LF to the wire when the user presses enter on the keyboard.

## Linux Screen Utility

In order to get the required CR LF line endingings on the wire with the screen utility add the line

`bindkey "\015" stuff "\015\012"`

To the .screenrc file in the home directory. Create the file if necessary.

## Other Utilities

Add additional notes here for tools that have been used for using terminal.