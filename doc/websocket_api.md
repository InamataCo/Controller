# WebSocket API

## Peripheral Parameters

The add command creates a new peripheral. It has to be called before the execute command can be sent to a created peripheral. Each add command has to have the following JSON parameters in addition to peripheral specific parameters.

| parameter | content                     |
| --------- | --------------------------- |
| uuid      | unique ID of the peripheral |
| type      | type of the peripheral      |

### Dummy Peripheral

No extra parameters required

### LED Peripheral

The peripheral supports the *SetValue* capability for which *WriteActuator* is the simplest task to set a value. Expects a value between 0 and 1.

| parameter       | content                                            |
| --------------- | -------------------------------------------------- |
| pin             | PWM output pin                                     |
| data_point_type | UUID of the data point type setting the brightness |

### I2C Adapter Peripheral

| parameter | content          |
| --------- | ---------------- |
| scl       | clock signal pin |
| sda       | data signal pin  |

### BH1750 Peripheral

| parameter   | content                            |
| ----------- | ---------------------------------- |
| i2c_adapter | name of the I2C adapter peripheral |
| address     | address of the BH1750 sensor       |

Note: The I2C adapter has to have been added

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
  peripheral: "...",
  <time: "...",>
  data_points: [
    {
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
    status: <"fail", "updating", "finish", "success">,
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
