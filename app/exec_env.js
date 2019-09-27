class Color {
  constructor(text) {
    this.text = text;
  }

  toString() {
    return this.text;
  }
}

function restrict(value, min, max) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  } else {
    return value;
  }
}

function asNumber(value) {
  const result = Number(value);
  if (isNaN(result)) {
    throw new TypeError("Expecting a number");
  } else {
    return result;
  }
}

function assertArray(value) {
  if (value instanceof Array) {
    return value;
  } else {
    throw new TypeError("Expecting an array");
  }
}

function assertColor(value) {
  if (value instanceof Color) {
    return value;
  } else {
    throw new TypeError("Expecting a color");
  }
}

async function sleep(time) {
  time = asNumber(time);
  return new Promise(accept => {
    setTimeout(accept, time * 1000);
  });
}

function time() {
  return Date.now() / 1000;
}

function* range(first, second = undefined, third = undefined) {
  var begin = 0,
    end,
    step = 1;
  if (second != undefined) {
    begin = asNumber(first);
    end = asNumber(second);
    if (third != undefined) {
      step = asNumber(third);
    }
  } else {
    end = asNumber(first);
  }
  for (var i = begin; step >= 0 ? i < end : i > end; i += step) {
    yield i;
  }
}

function print(obj) {
  webkit.messageHandlers.system.postMessage({
    type: "log",
    message: String(obj)
  });
}

const ArrayUtils = {
  length(list) {
    assertArray(list);
    return list.length;
  },

  append(list, element) {
    assertArray(list);
    list.push(element);
  },

  insert(list, index, element) {
    assertArray(list);
    index = asNumber(index);
    list.splice(index, 0, element);
  },

  remove(list, index) {
    assertArray(list);
    index = asNumber(index);
    list.splice(index, 1);
  }
};

const MathUtils = {
  RAD_PER_DEG: Math.PI / 180,

  random(min, max) {
    min = asNumber(min);
    max = asNumber(max);
    return min + Math.random() * (max - min);
  },

  abs(value) {
    value = asNumber(value);
    return Math.abs(value);
  },

  sqrt(value) {
    value = asNumber(value);
    return Math.sqrt(value);
  },

  sin(degree) {
    degree = asNumber(degree);
    return Math.sin(degree * this.RAD_PER_DEG);
  },

  cos(degree) {
    degree = asNumber(degree);
    return Math.cos(degree * this.RAD_PER_DEG);
  },

  tan(degree) {
    degree = asNumber(degree);
    return Math.tan(degree * this.RAD_PER_DEG);
  },

  asin(value) {
    value = asNumber(value);
    return Math.asin(value) / this.RAD_PER_DEG;
  },

  acos(value) {
    value = asNumber(value);
    return Math.acos(value) / this.RAD_PER_DEG;
  },

  atan(value) {
    value = asNumber(value);
    return Math.atan(value) / this.RAD_PER_DEG;
  },

  ln(value) {
    value = asNumber(value);
    return Math.log(value);
  },

  log(value) {
    value = asNumber(value);
    return Math.log10(value);
  },

  round(value) {
    value = asNumber(value);
    return Math.round(value);
  },

  floor(value) {
    value = asNumber(value);
    return Math.floor(value);
  },

  ceil(value) {
    value = asNumber(value);
    return Math.ceil(value);
  }
};

const ColorUtils = {
  rgb(red, green, blue) {
    red = restrict(asNumber(red), 0, 255);
    green = restrict(asNumber(green), 0, 255);
    blue = restrict(asNumber(blue), 0, 255);

    return new Color("rgb(" + red + "," + green + "," + blue + ")");
  },

  rgba(red, green, blue, alpha) {
    red = restrict(asNumber(red), 0, 255);
    green = restrict(asNumber(green), 0, 255);
    blue = restrict(asNumber(blue), 0, 255);
    alpha = restrict(asNumber(alpha), 0, 1);

    return new Color(
      "rgba(" + red + "," + green + "," + blue + "," + alpha + ")"
    );
  },

  hsl(hue, saturation, lightness) {
    hue = restrict(asNumber(hue), 0, 360);
    saturation = restrict(asNumber(saturation), 0, 1) * 100;
    lightness = restrict(asNumber(lightness), 0, 1) * 100;

    return new Color("hsl(" + hue + "," + saturation + "%," + lightness + "%)");
  },

  hsla(hue, saturation, lightness, alpha) {
    hue = restrict(asNumber(hue), 0, 360);
    saturation = restrict(asNumber(saturation), 0, 1) * 100;
    lightness = restrict(asNumber(lightness), 0, 1) * 100;
    alpha = restrict(asNumber(alpha), 0, 1);

    return new Color(
      "hsla(" + hue + "," + saturation + "%," + lightness + "%," + alpha + ")"
    );
  }
};

const Graphics = {
  stroke: true,
  fill: true,

  captureContext(canvas) {
    const newContext = canvas.getContext("2d");
    if (this.context == undefined) {
      // Default style
      newContext.strokeStyle = "rgba(255,255,255,1)";
      newContext.fillStyle = "rgba(0,0,0,1)";
      newContext.lineWidth = 1;
    } else {
      newContext.strokeStyle = this.context.strokeStyle;
      newContext.fillStyle = this.context.fillStyle;
      newContext.lineWidth = this.context.lineWidth;
      newContext.lineCap = this.context.lineCap;
      newContext.lineJoin = this.context.lineJoin;
      newContext.miterLimit = this.context.miterLimit;
      newContext.lineDashOffset = this.context.lineDashOffset;
      newContext.setLineDash(this.context.getLineDash());
    }
    this.context = newContext;
  },

  origin() {
    return {
      x: this.context.canvas.width / 2,
      y: this.context.canvas.height / 2
    };
  },

  async drawLine(startX, startY, endX, endY) {
    startX = asNumber(startX);
    startY = asNumber(startY);
    endX = asNumber(endX);
    endY = asNumber(endY);

    const origin = this.origin();
    this.context.beginPath();
    this.context.moveTo(startX + origin.x, startY + origin.y);
    this.context.lineTo(endX + origin.x, endY + origin.y);
    this.context.stroke();
    await sleep(0);
  },

  async drawArc(x, y, radius, startAngle, endAngle) {
    x = asNumber(x);
    y = asNumber(y);
    radius = asNumber(radius);
    startAngle = asNumber(startAngle) - 90;
    endAngle = asNumber(endAngle) - 90;

    const origin = this.origin();
    this.context.beginPath();
    this.context.arc(
      x + origin.x,
      y + origin.y,
      radius,
      startAngle * MathUtils.RAD_PER_DEG,
      endAngle * MathUtils.RAD_PER_DEG
    );
    this.context.stroke();
    await sleep(0);
  },

  async drawRect(x, y, width, height) {
    x = asNumber(x);
    y = asNumber(y);
    width = asNumber(width);
    height = asNumber(height);

    const origin = this.origin();
    this.context.beginPath();
    this.context.rect(x + origin.x, y + origin.y, width, height);
    this.context.stroke();
    this.context.fill();
    await sleep(0);
  },

  async drawEllipse(x, y, hRadius, vRadius) {
    x = asNumber(x);
    y = asNumber(y);
    hRadius = asNumber(hRadius);
    vRadius = asNumber(vRadius);

    const origin = this.origin();
    this.context.beginPath();
    this.context.ellipse(
      x + origin.x,
      y + origin.y,
      hRadius,
      vRadius,
      0,
      0,
      Math.PI * 2
    );
    this.context.stroke();
    this.context.fill();
    await sleep(0);
  },

  clearCanvas(color) {
    assertColor(color);

    document.body.style.backgroundColor = color.text;
    this.context.clearRect(
      0,
      0,
      this.context.canvas.width,
      this.context.canvas.height
    );
  },

  setLineWidth(width) {
    width = asNumber(width);
    this.context.lineWidth = width;
  },

  setLineColor(color) {
    assertColor(color);
    this.context.strokeStyle = color.text;
  },

  setFillColor(color) {
    assertColor(color);
    this.context.fillStyle = color.text;
  }
};

const Logo = {
  x: 0,
  y: 0,
  dir: Math.PI,
  isPenDown: true,

  async forward(length) {
    length = asNumber(length);

    const originalX = this.x;
    const originalY = this.y;
    this.x -= length * Math.sin(this.dir);
    this.y += length * Math.cos(this.dir);
    if (this.isPenDown) {
      await Graphics.drawLine(originalX, originalY, this.x, this.y);
    }
  },

  async backward(length) {
    await this.forward(-asNumber(length));
  },

  turnRight(degree) {
    degree = asNumber(degree);
    this.dir = (this.dir + degree * MathUtils.RAD_PER_DEG) % (Math.PI * 2);
  },

  turnLeft(degree) {
    this.turnRight(-asNumber(degree));
  },

  penUp() {
    this.isPenDown = false;
  },

  penDown() {
    this.isPenDown = true;
  },

  async goHome() {
    if (this.isPenDown) {
      await Graphics.drawLine(this.x, this.y, 0, 0);
    }
    this.x = 0;
    this.y = 0;
    this.dir = Math.PI;
  }
};

window.env = {
  globals: {},

  asArray: assertArray,

  execute(func) {
    async function _exec() {
      try {
        await func();
      } catch (err) {
        webkit.messageHandlers.system.postMessage({
          type: "error",
          name: err.name,
          message: err.message,
          stacktrace: err.stack
        });
      }
    }
    _exec();
  },

  sleep: sleep,
  time: time,
  range: range,
  print: print,

  ArrayUtils: ArrayUtils,
  MathUtils: MathUtils,
  ColorUtils: ColorUtils,
  Graphics: Graphics,
  Logo: Logo
};

window.onload = () => {
  let canvas = document.getElementsByTagName("canvas")[0];
  canvas.width = document.body.scrollWidth;
  canvas.height = document.body.scrollHeight;

  Graphics.captureContext(canvas);

  let resizeEnd;
  window.onresize = () => {
    clearTimeout(resizeEnd);
    resizeEnd = setTimeout(() => {
      const newCanvas = document.createElement("canvas");
      newCanvas.width = document.body.scrollWidth;
      newCanvas.height = document.body.scrollHeight;

      Graphics.captureContext(newCanvas);
      Graphics.context.drawImage(
        canvas,
        (newCanvas.width - canvas.width) / 2,
        (newCanvas.height - canvas.height) / 2
      );

      document.body.insertBefore(newCanvas, canvas);
      document.body.removeChild(canvas);
      canvas = newCanvas;
    }, 100);
  };
};
