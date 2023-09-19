# WebSocket API

## Controller Messages

### Command

```
{
  type: "cmd",
  request_id: "...",
  peripheral: {
    add: [
      {
        uuid: "",
        ...
      }
    ],
    remove: [
      {
        uuid: ""
      }
    ]
  },
  task: {
    start: [
      {
        uuid: "",
        ...
      }
    ],
    stop: [
      {
        uuid: ""
      }
    ],
    status: True
  },
  update: {
    url: "",
    size: int
  }
}
```

The server translates `run_until` parameters for task start commands to `duration_ms` parameters. This is due to lacking datetime arithmetic on the controllers and the need to be able to restart tasks on errors. Therefore, sending the server `duration_ms` will result in an error.

### Telemetry

```
{
  type: "tel",
  task: "...",
  peripheral: "...",
  <time: "...",>
  data_points: [
    {
      <time: "...",>
      value: 0-9,
      data_point_type: "..."
    }
  ]
}
```

### Results

```
{
  type: "result",
  <request_id: "...">
  peripheral: {
    add: [
      {
        uuid: "...",
        status: <"success", "fail">,
        <detail: "...">
      }
    ],
    remove: [
      {
        uuid: "...",
        status: <"success", "fail">,
        <detail: "...">
      }
    ]
  }
  task: {
    start: [
      {
        uuid: "...",
        status: <"success", "fail">,
        <detail: "...">
      }
    ],
    stop: [
      {
        uuid: "...",
        status: <"success", "fail">,
        <detail: "...">
      }
    ]
  },
  update: {
    status: <"fail", "updating", "finish">,
    detail: "..."
  }
}
```

### Register

```
{
  type: "reg",
  <peripherals: [uuid, ...]>,
  <tasks: [uuid, ...]>
  version: "..."
```

### System

```
{
  type: "sys",
  ...
}
```

## Tasks

### Start: `tasks/<uuid>/start`

The start command starts a new task. Each start command has to have the following JSON parameters in addition to peripheral specific parameters.

| parameter  | content                       |
| ---------- | ----------------------------- |
| type       | type of the peripheral        |
| peripheral | unique name of the peripheral |

On successful creation of the task, the following JSON is returned. In order to stop a long running task, its ID has to be stored on creation and then sent when it is to be stopped. The _type_ corresponds to the task's type while the _peripheral_ equals the name of the peripheral being used by the task. This may also be null.

| parameter  | content                           |
| ---------- | --------------------------------- |
| id         | unique ID of the task             |
| type       | type of the task                  |
| peripheral | name of the peripheral being used |

### Stop: `tasks/<uuid>/stop`

In order to stop a task, send its ID to the stop topic.

| parameter | content               |
| --------- | --------------------- |
| id        | unique ID of the task |
