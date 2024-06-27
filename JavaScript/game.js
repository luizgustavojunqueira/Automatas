let rows;
let cols;
let currentState;
let cellSize = 50;
let simRunning;
let simInterval;
let simSpeed = 1;
let width = 0;
let height = 0;

function initializeState() {
  let state = new Array(rows);

  for (let i = 0; i < rows; i++) {
    state[i] = new Array(cols);
    for (let j = 0; j < cols; j++) {
      state[i][j] = 0;
    }
  }

  return state;
}

function drawGrid(ctx) {
  ctx.fillStyle = "black";
  ctx.clearRect(0, 0, width, height);
  ctx.fillRect(0, 0, width, height);

  ctx.beginPath();

  for (let i = 1; i < cols; i++) {
    ctx.moveTo(i * cellSize, 0);
    ctx.lineTo(i * cellSize, height);
  }
  for (let i = 1; i < rows; i++) {
    ctx.moveTo(0, i * cellSize);
    ctx.lineTo(width, i * cellSize);
  }

  ctx.strokeStyle = "white";
  ctx.stroke();
}

function getMousePos(ctx, e) {
  let rect = ctx.canvas.getBoundingClientRect();

  let x = Math.floor(e.clientX - rect.left);
  let y = Math.floor(e.clientY - rect.top);

  x = x - (x % cellSize);
  y = y - (y % cellSize);

  return [x, y];
}

function drawCell(ctx, e) {
  if (!simRunning) {
    let [x, y] = getMousePos(ctx, e);

    currentState[y / cellSize][x / cellSize] =
      currentState[y / cellSize][x / cellSize] == 1 ? 0 : 1;
    drawState(ctx, currentState);
  }
}

function drawState(ctx, state) {
  drawGrid(ctx);
  ctx.fillStyle = "#f0c";

  for (let i = 0; i < rows; i++) {
    for (let j = 0; j < cols; j++) {
      if (state[i][j] == 1) {
        ctx.fillRect(j * cellSize, i * cellSize, cellSize, cellSize);
      }
    }
  }
}

function countNeighbors(state, x, y) {
  let sum = -state[x][y];

  for (let i = -1; i < 2; i++) {
    for (let j = -1; j < 2; j++) {
      sum += state[(x + i + rows) % rows][(y + j + cols) % cols];
    }
  }

  return sum;
}

function simulate(state, ctx) {
  drawGrid(ctx);

  let newState = initializeState(cellSize);

  for (let i = 0; i < rows; i++) {
    for (let j = 0; j < cols; j++) {
      let numN = countNeighbors(state, i, j);
      if (state[i][j] == 1) {
        if (numN < 2) {
          newState[i][j] = 0;
        } else if (numN < 4) {
          newState[i][j] = 1;
        } else {
          newState[i][j] = 0;
        }
      } else {
        if (numN === 3) {
          newState[i][j] = 1;
        }
      }
    }
  }

  return newState;
}

function startSimulation(ctx) {
  if (!simRunning) {
    simRunning = true;

    simInterval = setInterval(() => {
      currentState = simulate(currentState, ctx);
      drawGrid(ctx);
      drawState(ctx, currentState);
    }, simSpeed);
  }
}

function stopSimulation() {
  simInterval = clearInterval(simInterval);
  simRunning = false;
}

function resizeCanvasToDisplaySize(canvas) {
  // look up the size the canvas is being displayed
  width = canvas.clientWidth;
  height = canvas.clientHeight;

  width = width - (width % cellSize);
  height = height - (height % cellSize);
  console.log(width, height);

  // If it's resolution does not match change it
  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
    return true;
  }

  return false;
}

function generateRandomState() {
  let randomState = initializeState();

  for (let i = 0; i < rows; i++) {
    for (let j = 0; j < cols; j++) {
      randomState[i][j] = Math.floor(Math.random() + 0.2);
    }
  }

  return randomState;
}

function startGame() {
  // Get the canvas element
  const canvas = document.getElementById("gameCanvas");
  const ctx = canvas.getContext("2d");
  resizeCanvasToDisplaySize(ctx.canvas);

  rows = height / cellSize;
  cols = width / cellSize;

  drawGrid(ctx);

  currentState = initializeState(cellSize);

  canvas.addEventListener("mousedown", (e) => {
    drawCell(ctx, e);
  });

  document.getElementById("start").addEventListener("click", () => {
    startSimulation(ctx);
  });
  document.getElementById("stop").addEventListener("click", () => {
    stopSimulation();
  });

  document.getElementById("random").addEventListener("click", () => {
    currentState = generateRandomState();
    drawState(ctx, currentState);
  });
}

startGame();
