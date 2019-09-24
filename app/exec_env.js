function as_number(value) {
  const result = Number(value);
  if (isNaN(result)) {
    throw new TypeError("Expecting a number");
  } else {
    return result;
  }
}

function assert_array(value) {
  if (value instanceof Array) {
    return value;
  } else {
    throw new TypeError("Expecting an array");
  }
}

window.env = {
  globals: {},

  as_array: assert_array,

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

  async sleep(time) {
    time = as_number(time);
    return new Promise(accept => {
      setTimeout(accept, time * 1000);
    });
  },

  time() {
    return Date.now() / 1000;
  },

  *["range"](first, second = undefined, third = undefined) {
    var begin = 0,
      end,
      step = 1;
    if (second != undefined) {
      begin = as_number(first);
      end = as_number(second);
      if (third != undefined) {
        step = as_number(third);
      }
    } else {
      end = as_number(first);
    }
    for (var i = begin; step >= 0 ? i < end : i > end; i += step) {
      yield i;
    }
  },

  print(obj) {
    webkit.messageHandlers.system.postMessage({
      type: "log",
      message: String(obj)
    });
  },

  array_utils: {
    length(list) {
      assert_array(list);
      return list.length;
    },

    append(list, element) {
      assert_array(list);
      list.push(element);
    },

    insert(list, index, element) {
      assert_array(list);
      index = as_number(index);
      list.splice(index, 0, element);
    },

    remove(list, index) {
      assert_array(list);
      index = as_number(index);
      list.splice(index, 1);
    }
  },

  math_utils: {
    RAD_PER_DEG: Math.PI / 180,

    abs(value) {
      value = as_number(value);
      return Math.abs(value);
    },

    sqrt(value) {
      value = as_number(value);
      return Math.sqrt(value);
    },

    sin(degree) {
      degree = as_number(degree);
      return Math.sin(degree * this.RAD_PER_DEG);
    },

    cos(degree) {
      degree = as_number(degree);
      return Math.cos(degree * this.RAD_PER_DEG);
    },

    tan(degree) {
      degree = as_number(degree);
      return Math.tan(degree * this.RAD_PER_DEG);
    },

    asin(value) {
      value = as_number(value);
      return Math.asin(value) / this.RAD_PER_DEG;
    },

    acos(value) {
      value = as_number(value);
      return Math.acos(value) / this.RAD_PER_DEG;
    },

    atan(value) {
      value = as_number(value);
      return Math.atan(value) / this.RAD_PER_DEG;
    },

    ln(value) {
      value = as_number(value);
      return Math.log(value);
    },

    log(value) {
      value = as_number(value);
      return Math.log10(value);
    },

    round(value) {
      value = as_number(value);
      return Math.round(value);
    },

    floor(value) {
      value = as_number(value);
      return Math.floor(value);
    },

    ceil(value) {
      value = as_number(value);
      return Math.ceil(value);
    }
  },

  graphics: {
    captureContext(canvas) {
      const newContext = canvas.getContext("2d");
      if (this.context == undefined) {
        // Default style
        newContext.strokeStyle = "#ffffff";
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
      startX = as_number(startX);
      startY = as_number(startY);
      endX = as_number(endX);
      endY = as_number(endY);

      const origin = this.origin();
      this.context.beginPath();
      this.context.moveTo(startX + origin.x, startY + origin.y);
      this.context.lineTo(endX + origin.x, endY + origin.y);
      this.context.stroke();
      await sleep(0);
    },

    setLineWidth(width) {
      width = as_number(width);

      this.context.lineWidth = width;
    }
  },

  logo: {
    x: 0,
    y: 0,
    dir: Math.PI,
    isPenDown: true,

    async forward(length) {
      length = as_number(length);

      const originalX = this.x;
      const originalY = this.y;
      this.x -= length * Math.sin(this.dir);
      this.y += length * Math.cos(this.dir);
      if (this.isPenDown) {
        await graphics.drawLine(originalX, originalY, this.x, this.y);
      }
    },

    async backward(length) {
      await this.forward(-length);
    },

    turnLeft(degree) {
      degree = as_number(degree);

      this.dir = (this.dir + degree * math_utils.RAD_PER_DEG) % (Math.PI * 2);
    },

    turnRight(degree) {
      this.turnLeft(-degree);
    },

    penUp() {
      this.isPenDown = false;
    },

    penDown() {
      this.isPenDown = true;
    }
  }
};

window.onload = () => {
  let canvas = document.getElementsByTagName("canvas")[0];
  canvas.width = document.body.scrollWidth;
  canvas.height = document.body.scrollHeight;

  graphics.captureContext(canvas);

  let resizeEnd;
  window.onresize = () => {
    clearTimeout(resizeEnd);
    resizeEnd = setTimeout(() => {
      const newCanvas = document.createElement("canvas");
      newCanvas.width = document.body.scrollWidth;
      newCanvas.height = document.body.scrollHeight;

      graphics.captureContext(newCanvas);
      graphics.context.drawImage(
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
