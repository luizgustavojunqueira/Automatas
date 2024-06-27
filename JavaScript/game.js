let rows;
let cols;
let currentState;
let cellSize = 10;
let simRunning;
let simInterval;
let simSpeed = 5;

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

function drawGrid(canvas, ctx) {
  ctx.fillStyle = "black";
  ctx.clearRect(0, 0, canvas.clientWidth, canvas.clientHeight);
  ctx.fillRect(0, 0, canvas.clientWidth, canvas.clientHeight);

  ctx.beginPath();

  for (let i = 1; i < rows; i++) {
    ctx.moveTo(i * cellSize, 0);
    ctx.lineTo(i * cellSize, 800);
  }
  for (let i = 1; i < cols; i++) {
    ctx.moveTo(0, i * cellSize);
    ctx.lineTo(800, i * cellSize);
  }

  ctx.strokeStyle = "red";
  ctx.stroke();
}

function getMousePos(canvas, e) {
  let rect = canvas.getBoundingClientRect();

  let x = Math.floor(e.clientX - rect.left);
  let y = Math.floor(e.clientY - rect.top);

  x = x - (x % cellSize);
  y = y - (y % cellSize);

  return [x, y];
}

function drawCell(canvas, ctx, e) {
  if (!simRunning) {
    let [x, y] = getMousePos(canvas, e);

    currentState[y / cellSize][x / cellSize] =
      currentState[y / cellSize][x / cellSize] == 1 ? 0 : 1;
    drawState(ctx, currentState, canvas);
  }
}

function drawState(ctx, state, canvas) {
  drawGrid(canvas, ctx);
  ctx.fillStyle = "yellow";

  for (let i = 0; i < state.length; i++) {
    for (let j = 0; j < state.length; j++) {
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

function simulate(state, canvas, ctx) {
  drawGrid(canvas, ctx);

  let newState = initializeState(
    cellSize,
    canvas.clientWidth,
    canvas.clientHeight,
  );

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

function startSimulation(canvas, ctx) {
  simRunning = true;

  simInterval = setInterval(() => {
    currentState = simulate(currentState, canvas, ctx);
    drawGrid(canvas, ctx);
    drawState(ctx, currentState, canvas);
  }, simSpeed);
}

function stopSimulation() {
  simInterval = clearInterval(simInterval);
  simRunning = false;
}

function startGame() {
  // Get the canvas element
  const canvas = document.getElementById("gameCanvas");

  const ctx = canvas.getContext("2d");

  rows = canvas.clientHeight / cellSize;
  cols = canvas.clientWidth / cellSize;

  drawGrid(canvas, ctx);

  currentState = initializeState(
    cellSize,
    canvas.clientWidth,
    canvas.clientHeight,
  );

  canvas.addEventListener("mousedown", (e) => {
    drawCell(canvas, ctx, e);
  });

  document.getElementById("start").addEventListener("click", (e) => {
    startSimulation(canvas, ctx);
  });
  document.getElementById("stop").addEventListener("click", (e) => {
    stopSimulation();
  });
}

startGame();
