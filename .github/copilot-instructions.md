# BLE Attendance System - AI Coding Instructions

## Project Overview
This is a **single-page attendance management web application** for academic institutions. It uses BLE (Bluetooth Low Energy) technology to track student presence and generate reports on attendance patterns and irregularities. The entire application is built as a single HTML file with embedded CSS and JavaScript.

## Architecture & Key Patterns

### Single-Page Application (SPA) Design
- **Routing**: Hash-based navigation with `location.hash` and `hashchange` events (see `showSection()`)
- **Sections**: Three main views with IDs `section-login`, `section-dashboard`, `section-attendance`, `section-irregular`
- **Lazy initialization**: Attendance and irregularity filters only init when sections first display (flags: `attendanceInitialized`, `irregularInitialized`)
- **Navigation**: Tab-based nav buttons with `data-target` attributes trigger `showSection(key)`

### Authentication
- **Simple validation**: Hardcoded credentials in script (`validUser = "Principal"`, `validPass = "admin"`)
- **Session handling**: Login hides `section-login` and shows `app-container`; logout reverses this
- **No persistence**: Credentials are in-memory only

### Data & Filtering
- **In-memory tables**: All data is static HTML `<table>` elements with no backend
- **Attendance table**: Columns are Date, Name, Dept, ID, Year, Period1-4 (P/A values)
- **Irregularities table**: Date, Name, Dept, ID, Year, Time, Image
- **Filter architecture differs by section**:
  - **Attendance**: Multi-select checkboxes per column with custom `activeFilters` object; columns sorted by date/number/alphabetically
  - **Irregularities**: Simple dropdown select per column (one value at a time)

### Styling Conventions
- **Tailwind CSS**: All styling via CDN and utility classes; minimal custom CSS (only `.col-filter-dropdown` for custom height/overflow)
- **Print-friendly**: `onclick="window.print()"` button; ensure tables remain readable in print styles
- **Responsive**: Flexbox layouts with `sm:` breakpoints; tables use `table-fixed` and `overflow-x-auto` for mobile scrolling

## Critical Development Workflows

### Adding Table Data
- Insert new `<tr>` rows directly into `<tbody>` (static HTML approach)
- Ensure column count matches headers and existing filter logic
- Attendance uses P/A codes; Irregularities use timestamps and image paths

### Extending Filters
- **Attendance filters**: Add new `<th>` with nested filter-toggle button and filter-dropdown div (see existing pattern around line 150-250)
- **Irregularities filters**: Dropdown selects are auto-generated in `addIrrFilters()`, just update table headers
- Register new columns in `activeFilters` object if custom logic needed

### Adding New Sections
1. Create new `<section id="section-{name}">` inside `app-container`
2. Add to `sectionMap` object in navigation logic
3. Add nav button with `data-target="{name}"`
4. Initialize lazy logic in `showSection()` if needed

## Project-Specific Conventions

### Naming & IDs
- Element IDs use kebab-case: `section-login`, `attendanceTable`, `applyRangeBtn`
- Data attributes for navigation: `data-target`, `data-col` (column index)
- CSS classes: Tailwind utilities + custom `.col-filter-dropdown`, filter UI uses `.filter-toggle`, `.filter-dropdown`

### Event Delegation
- Click handlers on table for filter dropdowns use `document.addEventListener('click')` to avoid event bubbling
- Filter buttons stop propagation: `e.stopPropagation()`

### String Sanitization
- Checkbox IDs sanitize user values: `String(val).replace(/[^a-z0-9]/gi, '_')` to avoid special char conflicts

### Table Column Indexing
- Filters reference columns by numeric index (`colIndex`) from `querySelectorAll('th')` order
- This is fragile: adding/removing columns requires updating filter logic column references

## External Dependencies & Integration Points

- **Tailwind CSS CDN**: https://cdn.tailwindcss.com (required for styling)
- **No backend**: Application is fully client-side; no API calls or server integration
- **Static assets**: Image files referenced directly (`student_skipping_1.png`, etc.) must exist in root directory
- **Print API**: Uses browser's native print dialog (`window.print()`)
- **localStorage**: Not currently used; consider for persistent sessions/filters if extending

## Key Files & Their Responsibilities

- [index.html](../index.html): Complete application (722 lines) - all HTML, CSS, JavaScript
  - Lines 1-14: Meta tags, Tailwind CDN, custom CSS
  - Lines 21-45: Login form markup
  - Lines 47-722: App container, three main sections, combined JavaScript logic
- [README.md](../README.md): Deployment instructions (GitHub Pages) and feature overview

## Testing & Debugging Tips

- Open browser DevTools **Console** to check filter state: `activeFilters` object tracks current filter selections
- Test filter logic by examining `filterTable()` function - it iterates `activeFilters` and hides rows
- Use hash navigation for direct section access: `#attendance`, `#irregular`, `#dashboard`
- Attendance data is hard-coded in rows 350-374; modify there for testing

---

**Last Updated**: January 2, 2026  
**Deployment**: GitHub Pages via root `/` directory
