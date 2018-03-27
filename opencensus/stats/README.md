# Stats API

The primary stats API documentation is in the headers in `opencensus/stats`.
See the following classes and headers for the top-level interfaces:

### Recording data
- A [`Measure`](measure.h) specifyies the resources against which data is
  recorded.
- [`recording.h`](recording.h) defines the recording function.

### Accessing data
- A [`ViewDescriptor`](view_descriptor.h) defines what data a view collects,
  and provides an interface for registering it for export.
- A [`View`](view.h) provides a handle for accessing data for a view within the
  task.
