# WIGMS
Wedding Invitation and Gift Management System - C Programming Project
1.  Overview

WIGMS (Wedding / Invitation Guest Management System) is a command-line and GUI C application that provides end-to-end management of guests, gifts, categories, and parking for weddings and other invitation-based events. The project is built entirely in C (C11) and is organised as a collection of independent but interoperable modules, each with its own header and implementation file.

Key capabilities at a glance:

Register, search, update, and delete guest (Person) records with RSVP tracking

Organise guests into named categories (VIP, Family, Friend, Colleague, etc.)

Track gifts: add, update, delete, link to guests, export to CSV

Generate polished HTML reports with statistics and per-guest summaries

Manage parking spot assignments and vehicle information

GTK3 graphical interface for point-and-click operation

All data persisted to binary files for fast re-load between sessions


2.  Project Structure

The repository is organised as follows:

WIGMS/
├── main.c                 Entry point & main menu
├── person.h / person.c    Guest (Person) module
├── category.h / category.c  Category module
├── gift.h / gift.c        Gift Management module
├── report.h / report.c    HTML Report module
├── parking.h / parking.c  Parking Management module
├── gtk_ui.h / gtk_ui.c    GTK3 GUI module
├── data/                  Binary data files (auto-created at runtime)
│   ├── persons.dat
│   ├── categories.dat
│   ├── gifts.dat
│   └── parking.dat
├── exports/               CSV exports
├── reports/               Generated HTML reports
├── Makefile               Build configuration
└── README.md              This file

3.  Modules

The table below summarises each module, its source files, and its primary responsibility.

Module
Source File(s)
Responsibility
Person
person.c / person.h
Add, list, search, update, delete guests; binary file I/O
Category
category.c / category.h
Manage guest categories (VIP, Family, Friend, etc.)
Gift
gift.c / gift.h
Gift CRUD, link gifts to guests, CSV export, binary persistence
Report
report.c / report.h
HTML report generation, statistics, per-guest & global summaries
Parking
parking.c / parking.h
Assign parking spots, track vehicles, availability check
GTK UI
gtk_ui.c / gtk_ui.h
GTK3 graphical interface wrapping all modules
Main
main.c
Entry point; initialises modules, displays main menu


3.1  Person Module

The Person module is the core of WIGMS. Every guest is represented as a Person struct stored in persons.dat. The module provides full CRUD via a sub-menu and supports fuzzy name search, phone/email lookup, and RSVP status management.

Person struct fields:

Field
Type
Description
person_id
int
Auto-incremented unique identifier
first_name
char[]
Guest first name
last_name
char[]
Guest last name
phone
char[]
Contact phone number
email
char[]
Contact email address
category_id
int
Link to a Category record
rsvp_status
char[]
RSVP state: Confirmed, Pending, or Declined


3.2  Category Module

Categories allow the event organiser to segment guests. Each category has a unique ID, a name (e.g. VIP, Family, Friend, Colleague, Other), and an optional description. Guests are assigned a category_id when created or updated. The module persists data in categories.dat.

3.3  Gift Management Module

The Gift module tracks presents brought by guests. Each gift record is linked to exactly one guest via guest_id and includes a value estimate and a status indicator.

Gift struct fields:

Field
Type
Description
gift_id
int
Auto-incremented unique identifier
guest_id
int
Foreign key linking gift to a guest (Person)
gift_name
char[]
Descriptive name of the gift
gift_value
float
Estimated monetary value
gift_status
char[]
Current status: Received, Pending, or Returned


Module features:

Add Gift — prompts for guest ID, name, value, and status; auto-assigns gift_id

List All Gifts — displays all gifts in a formatted table

Search by Guest ID — shows all gifts belonging to a specific guest

Update Gift — modify name, value, or status of an existing record

Delete Gift — remove a gift by ID with confirmation prompt

Export to CSV — writes all gifts to exports/gifts_export.csv for spreadsheet use

Binary persistence — gifts.dat written on every change; loaded at startup


3.4  Report Module

The Report module generates a self-contained HTML file (reports/report.html) containing:

Event-level statistics: total guests, confirmed / pending / declined counts, total gift value

Category breakdown table

Per-guest cards listing RSVP status, category, gifts, and parking assignment

Embedded CSS for a polished, printable layout

The report can be opened in any modern web browser. It is regenerated on demand from current binary data.

3.5  Parking Module

The Parking module assigns numbered parking spots to guests arriving by vehicle. It stores vehicle registration numbers, assigned spot numbers, and links back to guest IDs. Features include availability checking, spot reassignment, and release. Data persists in parking.dat.

3.6  GTK3 Graphical Interface

gtk_ui.c wraps all module functions behind a GTK3 window with a navigation sidebar and content panel. The GUI provides the same functionality as the CLI menus, allowing non-technical users to manage guests, gifts, categories, and parking through a point-and-click interface.

4.  Prerequisites

Before building WIGMS, ensure the following are installed on your system:

GCC (C11 support)  —  gcc --version should report 7.0 or later

Make  —  GNU Make for running the Makefile

GTK3 development libraries  —  required for the GUI module

On Debian / Ubuntu:
sudo apt-get update
sudo apt-get install build-essential libgtk-3-dev
On Fedora / RHEL:
sudo dnf install gcc make gtk3-devel
On macOS (Homebrew):
brew install gtk+3

5.  Building the Project

Clone the repository

git clone https://github.com/franckkaze4-collab/WIGMS.git
cd WIGMS

Compile

make
This produces the wigms executable in the project root.

Clean build artefacts

make clean

Manual compilation (without Make)

gcc -std=c11 -Wall -Wextra -o wigms main.c person.c category.c \
    gift.c report.c parking.c gtk_ui.c \
    $(pkg-config --cflags --libs gtk+-3.0)

6.  Running the Application

CLI (console menu)

./wigms
The main menu is presented on the terminal. Navigate with the number keys and press Enter to confirm each selection.

GTK GUI

./wigms --gui
Launches the GTK3 window. All modules are accessible from the sidebar.

7.  Data Persistence

All records are stored in binary (.dat) files located in the data/ directory. The directory is created automatically on first run. Each module loads its file at startup and flushes changes immediately after every write operation, ensuring data is never lost if the program exits unexpectedly.

Binary format details:

Records are written as raw structs using fwrite / fread

Deleted records are compacted by rewriting the file without the removed entry

Auto-increment IDs are derived from the maximum existing ID + 1 at load time


8.  HTML Report

Select Generate Report from the main menu (or click the Report button in the GUI) to produce reports/report.html. The report file is self-contained: all styles are embedded inline so it can be shared as a single file. Open it in any browser to view or print.

Report sections:

Summary statistics panel

Guest count by category (table)

Individual guest cards (RSVP, gifts, parking)

Total gift value summary


9.  CSV Export

The Gift module provides a dedicated CSV export. From the Gift Management sub-menu choose Export Gifts to CSV. The file is written to exports/gifts_export.csv with the following columns:

gift_id,guest_id,gift_name,gift_value,gift_status

The CSV file can be opened directly in Microsoft Excel, LibreOffice Calc, or any spreadsheet application for further analysis.

10.  Contributing

Contributions are welcome. Please follow the workflow below:

Fork the repository and create a feature branch: git checkout -b feature/your-feature

Write or modify code; ensure it compiles cleanly with no warnings under -Wall -Wextra

Test all affected modules manually (CLI and GUI)

Commit with a clear message: git commit -m "feat: short description"

Push and open a Pull Request against the main branch


Code style guidelines:

Follow the existing naming convention: snake_case for variables and functions

Keep functions short and focused (< 60 lines is a good guide)

Document every public function in its header file with a brief description

Avoid global variables; prefer passing context through function parameters


11.  Authors & Acknowledgements

Developed collaboratively under the franckkaze4-collab GitHub organisation. All contributors are listed in the repository's Contributors tab.

Special thanks to the open-source community behind the GTK project and the C standard library.

12.  License

This project is released under the MIT License. See the LICENSE file in the repository root for the full text. You are free to use, modify, and distribute this software with proper attribution.
