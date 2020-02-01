;(function() { // anonymous scope
	var $ = window['jQuery']
	
	//constants
	var COLUMNS = 'abcdefgh'.split('')
	var START_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR'
	var START_POSITION = fenToObj(START_FEN)
	var DEFAULT_MOVE_SPEED = 2000 // in ms
	var DEFAULT_DRAG_THROTTLE_RATE = 20 // in ms
	
	// unique class names
	var boardBorderSize = 2
	var CSS = {}
	CSS['white'] = 'black-3c85d'
	CSS['black'] = 'white-1e1d7'
	CSS['notation'] = 'notation-322f9'
	CSS['board'] = 'board-b72b1'
	CSS['chessboard'] = 'chessboard-63f37'
	CSS['highlight1'] = 'highlight1-32417'
    CSS['highlight2'] = 'highlight2-9c5d2'
	CSS['piece'] = 'piece-417db'
	CSS['row'] = 'row-5277c'
	CSS['square'] = 'square-55d63'
	CSS['notation'] = 'notation-322f9'
	CSS['clearfix'] = 'clearfix-7da63'
	CSS['alpha'] = 'alpha-d2270'
	CSS['numeric'] = 'numeric-fc462'
	
	function throttle(f, scope) {
	    var timeout = 0
		var shouldFire = false
		var args = []
		
		var handleTimeout = function() {
		    timeout = 0
			if (shouldFire) {
			    shouldFire = false
				fire()
			}
		}
		
		var fire = function() {
		    timeout = window.setTimeout(handleTimeout, DEFAULT_DRAG_THROTTLE_RATE)
			f.apply(scope, args)
		}
		
		return function(_args) {
		   args = arguments
		   if (!timeout) {
		       fire()
		   } else {
		       shouldFire = true
		   }
		}
	}
	
	
    function generateUniqueId() {
		return 'xxxx-xxxx-xxxx-xxxx-xxxx-xxxx-xxxx-xxxx'.replace(/x/g, function(c) {
			var r = (Math.random() * 16) | 0
			return r.toString(16)
		})
	}
	
	function deepCopy(thing) {
		return JSON.parse(JSON.stringify(thing))
	}
	
	function squeezeFenEmptySquares(fen) {
		return fen.replace(/11111111/g, '8')
		    .replace(/1111111/g, '7')
			.replace(/111111/g, '6')
			.replace(/11111/g, '5')
			.replace(/1111/g, '4')
			.replace(/111/g, '3')
			.replace(/11/g, '2')
	}
	
	function expandFenEmptySquares (fen) {
		return fen.replace(/8/g, '11111111')
		  .replace(/7/g, '1111111')
		  .replace(/6/g, '111111')
		  .replace(/5/g, '11111')
		  .replace(/4/g, '1111')
		  .replace(/3/g, '111')
		  .replace(/2/g, '11')
    }
	
	function validFen(fen) {
	    // only position is checked the end: castling is cut off
		fen = fen.replace(/ .+$/, '')
		
		// expand the empty square numbers to just 1s
		fen = expandFenEmptySquares(fen)
		
		// FEN should have 8 sections separated by slashes
        var chunks = fen.split('/')
		if (chunks.length !== 8) return false
		
		//check each section
		for (var i = 0; i < 8; i++) {
		    if (chunks[i].length !== 8 || chunks[i].search(/[^kqrnbpKQRNBP1]/) !== -1) 
			{
				return false
			}
		}			
		
		return true
	}
	
	function fenToPieceCode(piece) {
        if (piece === "k") return 'blackKing'
		if (piece === "p") return 'blackPawn'
		if (piece === "n") return 'blackKnight'
		if (piece === "b") return 'blackBishop'
		if (piece === "r") return 'blackRook'
		if (piece === "q") return 'blackQueen'
        if (piece === "K") return 'whiteKing'
        if (piece === "P") return 'whitePawn'
        if (piece === "N") return 'whiteKnight'
        if (piece === "B") return 'whiteBishop'
        if (piece === "R") return 'whiteRook'
        if (piece === "Q") return 'whiteQueen'

        return '';		
    }
	
	function pieceCodeToFen(piece) {
        if (piece === "blackKing") return 'k'
		if (piece === "blackPawn") return 'p'
		if (piece === "blackKnight") return 'n'
		if (piece === "blackBishop") return 'b'
		if (piece === "blackRook") return 'r'
		if (piece === "blackQueen") return 'q'
        if (piece === "whiteKing") return 'K'
        if (piece === "whitePawn") return 'P'
        if (piece === "whiteKnight") return 'N'
        if (piece === "whiteBishop") return 'B'
        if (piece === "whiteRook") return 'R'
        if (piece === "whiteQueen") return 'Q'

        return '';		
    }
	
	function isString (s) {
        return typeof s === 'string'
    }
	
	function validSquare(square) {
	    return isString(square) && square.search(/^[a-h][1-8]$/) !== -1
	}
	
	function validPieceCode(piece) {
	    if (piece === "blackKing" || 
		    piece === "blackPawn" || 
	        piece === "blackKnight" ||
		    piece === "blackBishop" ||
		    piece === "blackRook" ||
		    piece === "blackQueen" ||
            piece === "whiteKing" ||
            piece === "whitePawn" ||
            piece === "whiteKnight" ||
            piece === "whiteBishop" ||
            piece === "whiteRook" ||
            piece === "whiteQueen") {
			return true
		} else {
			return false
		}
	}
	
	function validPositionObject(position) {
		for (var square in position) {
		    if (!position.hasOwnProperty(square)) continue
			
			if (!validPieceCode(position[square]) || !validSquare(square)) return false
		}
		
	    return true
	}
	
	// convert FEN string to position object
	// if FEN string invalid then it returns with false
	function fenToObj(fen) {
		if (!validFen(fen)) return false
		
		// cut off any move, castling, etc. info from the end
        // we are only interested in position information		
		fen = fen.replace(/ .+$/, '')
		
		var rows = fen.split('/')
		var position = {}
		
		var currentRow = 8
		
		for (var i = 0; i < 8; i++) {
		   var row = rows[i].split('')
		   var columnIndex = 0
		   
		   // walk through each character in FEN section
		   for (var j = 0; j < row.length; j++) {
		       // number / empty squares 
			   if (row[j].search(/[1-8]/) !== -1) {
				   var emptySquaresCount = parseInt(row[j], 10)
				   columnIndex += emptySquaresCount
			   } else {
			       // piece
				   var square = COLUMNS[columnIndex] + currentRow
                   position[square] = fenToPieceCode(row[j])				   
			       columnIndex++
			   }
		   }
		   
		   currentRow--
		}
		
		return position
	}
	
	// convert postion object to FEN string
	// returns false incase of invalid position object
	function objectToFen(object) {
		if (!validPositionObject(object)) return false
		
		var fen = ''
		
		var currentRow = 8
		for (var i = 0; i <  8; i++) {
			for (var j = 0; j <  8; j++) {
				var square = COLUMNS[j] + currentRow
				
				// piece exists
				if (object.hasOwnProperty(square)) {
					fen += pieceCodeToFen(object[square])
				} else { // empty square
					fen += '1'
				}
			}
				
			if (i !== 7) {
				fen += '/'
			}
			
			currentRow--
		}
		
        // squeeze the empty numbers together
        fen = squeezeFenEmptySquares(fen)

        return fen
	}		

    function squareDistance(square1, square2) {
	    var square1Array = square1.split('')
		var square1X = COLUMNS.indexOf(square1Array[0]) + 1
		var square1Y = parseInt(square1Array[1], 10)
		
		var square2Array = square2.split('')
		var square2X = COLUMNS.indexOf(square2Array[0]) + 1
		var square2Y = parseInt(square2Array[1], 10)
		
		var deltaX = Math.abs(square1X - square2X)
		var deltaY = Math.abs(square1Y - square2Y)
		
		if (deltaX > deltaY) {
			return deltaX 
		} else {
		    return deltaY
		}
	}
	
	// returns an array of closest squares from square
    function createRadius (square) {
		var squares = []

		// calculate distance of all squares
		for (var i = 0; i < 8; i++) {
		    for (var j = 0; j < 8; j++) {
			    var s = COLUMNS[i] + (j + 1)

			    // skip the square we're starting from
			    if (square === s) continue

			    squares.push({
			        square: s,
			        distance: squareDistance(square, s)
			    })
		    }
		}

		// sort by distance
		squares.sort(function (a, b) {
		    return a.distance - b.distance
		})

		// just return the square code
		var surroundingSquares = []
		for (i = 0; i < squares.length; i++) {
		     surroundingSquares.push(squares[i].square)
		}

		return surroundingSquares
    }
	
	// returns the square of the closest instance of piece 
	// returns false if no instance of piece is found in position
    function findClosestPiece(position, piece, square) {
		
		// create array of closest squares from 'square' 
		// closestSquares will contain remote square 'code'
		var closestSquares = createRadius(square)
	 
		for (var i = 0; i < closestSquares.length; i++) {
			var remoteSquare = closestSquares[i]
		    if (position.hasOwnProperty(remoteSquare) && position[remoteSquare] == piece) {
			    return remoteSquare
			}				
		}
			
		return false
	}		
	
	// constructor
	function constructor(containerName) { // containerName : id of the DOM node
		
		// DOM elements
		var $board = null
		var $draggedPiece = null
	
	    containerName = '#' + containerName
		var $container = $(containerName)
		
		// returning object of constructor
        var widget = {}
 
        // other variables
		var squareIds = {}
		var currentPosition = {}
        var isDragging = false
		var squareSize = 16
		var draggedPiece = null
		var draggedPieceSource = null
		var draggedPieceLocation = null
		var squareOffsets = {}
		var nextColor = "white"
		var positions = ['rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1']
		var castleAbility = 'KQkq'
		var enpassantTarget = '-'
		var stepCounter = 0
		
		function castlingToObject(castlingId, newPosition) {
            switch(castlingId) {
                case 0:
				    newPosition['f1'] = currentPosition['h1']
				    delete newPosition['h1']
                    break
                case 1:
				    newPosition['d1'] = currentPosition['a1']
				    delete newPosition['a1']
                    break
                case 2:
				    newPosition['f8'] = currentPosition['h8']
				    delete newPosition['h8']
                    break
                case 3:
				    newPosition['d8'] = currentPosition['a8']
				    delete newPosition['a8']
                    break
                default:
            }
		}
		
		function stepToObject(fromSquare, toSquare, fromPiece, toPiece, square1X, square2X, square1Y, square2Y) {
			var newPosition = deepCopy(currentPosition)
			
			if (fromPiece === 'whiteKing' && fromSquare === 'e1' && toSquare === 'g1') {
				castlingToObject(0, newPosition) 
			}
			
			if (fromPiece === 'whiteKing' && fromSquare === 'e1' && toSquare === 'c1') {
				castlingToObject(1, newPosition) 
			}
			
			if (fromPiece === 'blackKing' && fromSquare === 'e8' && toSquare === 'g8') {
				castlingToObject(2, newPosition) 
			}
			
			if (fromPiece === 'blackKing' && fromSquare === 'e8' && toSquare === 'c8') {
				castlingToObject(3, newPosition) 
			}
			
			// check enpassant
			if (fromPiece.includes('Pawn') && toPiece === undefined && Math.abs(square1X - square2X) === 1 && Math.abs(square1Y - square2Y) === 1) {
				delete newPosition[COLUMNS[square2X - 1] + square1Y]
			}
			
			newPosition[toSquare] = fromPiece
			delete newPosition[fromSquare]
			
			return newPosition 		
	    }	
	
		// calculate all the needed animations so as to get from position1 to position2
		function calculateAnimations(position1, position2) {
			// make copies of both
			position1 = deepCopy(position1)
			position2 = deepCopy(position2)
			
			var animations = []
			var squaresMovedTo = {}
			
			// remove pieces that are the same in both positions
			for (var square in position2) {
				if (!position2.hasOwnProperty(square)) continue

                if (position1.hasOwnProperty(square) && position1[square] === position2[square]) {
				   delete position1[square]
				   delete position2[square]
				}					
			}
			
			// find all the "move" animations
			for (var square in position2) {
				var closestPiece = findClosestPiece(position1, position2[square], square)
				
				if (closestPiece) {
				    animations.push({
						type: 'move',
						source: closestPiece,
						destination: square,
						piece: position2[square]
					})
				
	      			delete position1[closestPiece]
			    	delete position2[square]
				
                    squaresMovedTo[square] = true	
				}				
			}
			
			// find all "add" animations
			for (var currentSquare in position2) {
			    if (!position2.hasOwnProperty(currentSquare)) continue
			    
				animations.push({
						type: 'add',
						square: currentSquare,
						piece: position2[currentSquare]
					})
					
				delete position2[currentSquare]
			}
			
			// find "clear" animations
			for (var currentSquare in position1) {
				if (!position1.hasOwnProperty(currentSquare)) continue
			
			    // do not clear a piece if it's on a square that is the result
				// of a 'move', ie: piece capture
			    if (squaresMovedTo.hasOwnProperty(currentSquare)) continue 
				
			    animations.push({
						type: 'clear',
						square: currentSquare,
						piece: position1[currentSquare]
				})
					
			    delete position1[currentSquare]
			}
			
			return animations
		}
		
		function setCurrentPosition(position) {
	        var oldPostion = deepCopy(currentPosition)
			var newPosition = deepCopy(position)
			var oldFen = objectToFen(oldPostion)
			var newFen = objectToFen(newPosition)
			
			// no change in position 
			if (oldFen === newFen) return

            currentPosition = position			
     	}
		
		function animateSquareToSquare(source, destination, piece, completeFunction) {
			// get information about the source and destination square
			var $sourceSquare = $('#' + squareIds[source])
			var sourcePosition = $sourceSquare.offset()
			var $destinationSquare = $('#' + squareIds[destination])
			var destinationPosition = $destinationSquare.offset()
			
			// create the animated piece and absolutly position it
			// over the source sqare
			var animatedPieceId = generateUniqueId()
			$('body').append(buildPieceHtml(piece, true, animatedPieceId))	
			var $animatedPiece = $('#' + animatedPieceId)
			
			$animatedPiece.css({
			    display: '',
                position: 'absolute',
                top: sourcePosition.top,
                left: sourcePosition.left
			})
			
			// remove original piece from source square
			$sourceSquare.find('.' + CSS.piece).remove()
			
			function onFinishedAnimation1() {
			    // add the "real" piece to the destination square 
				$destinationSquare.append(buildPieceHtml(piece))
				
				// remove the animated square
				$animatedPiece.remove()
				
				// run complete function
				completeFunction()
			}
			
			// animate the piece to the destination square
			var options = {
                duration: 2000,
                complete: onFinishedAnimation1			
			}
			
			$animatedPiece.animate(destinationPosition, options)
		}
		
		function animateDraggedPieceToSquare(destination, completeFunction) {
			// get information about the source and destination square
			var $destinationSquare = $('#' + squareIds[destination])
			var destinationPosition = $destinationSquare.offset()
			
			function onFinishedAnimation1() {
			    // add the "real" piece to the destination square 
				$destinationSquare.append(buildPieceHtml(draggedPiece))
				
				// run complete function
				completeFunction()
			}
			
			// animate the piece to the destination square
			var options = {
                duration: 2000,
                complete: onFinishedAnimation1			
			}
			
			$draggedPiece.animate(destinationPosition, options)
		}
		
		// execute an array of animations
		function doAnimations(animations, oldPosition, newPosition) {
		    if (animations.length === 0) return
			
			var finishedCount = 0
			
			function onFinishedAnimation3() {
			    finishedCount++
				
				// exit if there is an unfinished animation
				if (finishedCount !== animations.length) return
				
				setCurrentPosition(newPosition)
				
				drawPositionInstant()
			}

            for (var i = 0; i < animations.length; i++) {
			    var animation = animations[i]
				
				// clear a piece
				if (animation.type === 'clear') {
				    $('#' + animation.square + '.' + CSS.piece)
				        .fadeOut(2000, onFinishedAnimation3)
					   
			    // add a piece without spare pieces - fade the piece onto the square 
				} else if(animation.type === 'add') {
				    $('#' + squareIds[animation.square])
					    .append(buildPieceHtml(animation.piece, true, animation.square))
						.find('.' + CSS.piece)
				        .fadeIn(4000, onFinishedAnimation3)
				
				// move piece from squareA to squareB  
				} else if(animation.type === 'move') {
				    animateSquareToSquare(animation.source, animation.destination, animation.piece, onFinishedAnimation3)
				}								
					
			}			
		}
		
		widget.positionBase = function(position) {
			var animations = calculateAnimations(currentPosition, position)
			doAnimations(animations, currentPosition, position)
		}			
		
		widget.position = function(position) {
		    if (position.toLowerCase() === 'start') {
			    position = deepCopy(START_POSITION)
			}
			 
			widget.positionBase(position)
		}
		
		widget.loadFenHelper = function(fen) {			
            var inputFen = $('#fenId').val()
			
			var positionPieces = inputFen.split(' ')
			
			if (positionPieces[1] === 'w') {
				nextColor = 'white'
			} else {
				nextColor = 'black'
			}				
			
			castleAbility = positionPieces[2]
			enpassantTarget = positionPieces[3]
			
			console.log(inputFen)
			var loadedPosition = fenToObj(inputFen) 
			widget.positionBase(loadedPosition)
		}
		
		widget.loadFen = function(fen) {
            var inputFen = $('#fenId').val()			
			positions = []
			positions.push(inputFen)
			
			widget.loadFenHelper(fen)
		}
		
		widget.loadStep = function(wasmResult) {
			if (wasmResult.substring(0, 4) === 'SMME') {
				$('#messageTable > tbody:last-child').append('<tr><td>Draw! Stalemated!</td></tr>')
				return
			}
			
			if (wasmResult.substring(0, 4) === 'CHME') {
				$('#messageTable > tbody:last-child').append('<tr><td>You win! Congratulation!</td></tr>')
				return
			}
			
			if (wasmResult.substring(4, 8) === 'SMEN') {
				$('#messageTable > tbody:last-child').append('<tr><td>Draw! Stalemated!</td></tr>')
			}
			
			if (wasmResult.substring(4, 8) === 'CHEN') {
				$('#messageTable > tbody:last-child').append('<tr><td>You lost! Sorry!</td></tr>')
			}
			
			var wasmResultArray = wasmResult.split(' ')
			
			var fromSquare = wasmResult.substring(0, 2 );
			var toSquare = wasmResult.substring(2, 4);
			
			var oldFromPiece = currentPosition[fromSquare]
			var fromPiece = oldFromPiece
			
			if (oldFromPiece === 'whitePawn' && toSquare.includes('8')) {
			    fromPiece = 'whiteQueen'
			}
			
			if (oldFromPiece === 'blackPawn' && toSquare.includes('1')) {
			    fromPiece = 'blackQueen'
			}
			
			var toPiece = currentPosition[toSquare]
			
			var square1Array = fromSquare.split('')
		    var square1X = COLUMNS.indexOf(square1Array[0]) + 1
		    var square1Y = parseInt(square1Array[1], 10)
			var square2Array = toSquare.split('')
		    var square2X = COLUMNS.indexOf(square2Array[0]) + 1
		    var square2Y = parseInt(square2Array[1], 10)
			
			saveFen(toSquare, fromPiece, square1X, square2X, square1Y, square2Y)
			var newPosition = stepToObject(fromSquare, toSquare, fromPiece, toPiece, square1X, square2X, square1Y, square2Y)
			widget.positionBase(newPosition)
			changeNextColor()
		}
		
		widget.start = function() {
			widget.position('start')
			if (nextColor.includes("black")) {
			    changeNextColor()
			}
			stepCounter = 0
		    $("#stepTable td").parent().remove()
			$("#stepTable2 td").parent().remove()
			
		    positions = []
			positions.push(deepCopy('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'))
			$('#fenId').val(positions[0])
		}
		
		function stepClicked(event) {
			var stepString = event.target.id 
			var indexString = stepString.substr(4, stepString.length)
			var positionIndex = parseInt(indexString, 10)
			$('#fenId').val(positions[positionIndex])
			widget.loadFenHelper(positions[positionIndex])
		}
		
		function validSide(piece) {
		    return (piece.includes("white") && nextColor.includes("white")) || 
			       (piece.includes("black") && nextColor.includes("black"))
		}		
         
		function hasSameColor(piece1, piece2) {
			if (piece1 === undefined || piece2 === undefined) return false
		    return (piece1.includes("white") && piece2.includes("white")) ||
			       (piece1.includes("black") && piece2.includes("black"))
		}
         
		function validPawnStep(square1X, square1Y, square2X, square2Y, isWhiteStep, toPiece) {			
			if (toPiece === undefined) {
				if (square1X === square2X) {
				    if ((isWhiteStep && (square1Y - square2Y) === -1) || (!isWhiteStep && (square1Y - square2Y) === 1)) return true
     		        if (Math.abs(square1Y - square2Y) === 2 && isWhiteStep && square1Y == 2 && currentPosition[COLUMNS[square1X - 1] + (square1Y + 1)] === undefined) return true
				    if (Math.abs(square1Y - square2Y) === 2 && !isWhiteStep && square1Y == 7  && currentPosition[COLUMNS[square1X - 1] + (square1Y - 1)] === undefined) return true
				}
				
				if (COLUMNS[square2X-1] + square2Y === enpassantTarget) {
					if (isWhiteStep) { // black is on enpassant target
						var enpassantTargetPiece = currentPosition[COLUMNS[square2X-1] + (square2Y - 1)]
						
						if (square2Y - square1Y === 1 && Math.abs(square1X - square2X) === 1) {
							return true
						}
					}
					else {
						var enpassantTargetPiece = currentPosition[COLUMNS[square2X-1] + (square2Y - 1)]
						
						if (square2Y - square1Y === -1 && Math.abs(square1X - square2X) === 1) {
							return true
						}
					}
				}
			}
			else {
				if((square1Y - square2Y) === -1 && Math.abs(square1X - square2X) === 1) return true
			    if ((square1Y - square2Y) === 1 && Math.abs(square1X - square2X) === 1) return true
			}
			
			return false
		}
        
		function hasInnerPiece(row, col, square1X, square1Y, square2X, square2Y) {
			var fromX = square1X + col
			var fromY = square1Y + row
			var toX = square2X 
			var toY = square2Y 
			
			if ((col !== 0 && toX === fromX) || (row !== 0 && toY === fromY)) return false
			
			while ((col !== 0 && fromX !== toX) || (row != 0 && fromY !== toY)) {
                if (currentPosition[COLUMNS[fromX - 1] + fromY] !== undefined) return true	
	
    			fromX += col
		     	fromY += row				
			}
			
			return false
		}
		
		function isAttackedByAnyPiece(isWhiteAttacked, attackedSquareX, attackedSquareY, position) {
			// check pawn attacks			
			if (isWhiteAttacked) {
				if (isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY + 1, 'blackPawna', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY + 1, 'blackPawn', position) ) {
					    return true
				}
			} else {
				if (isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY - 1, 'whitePawn', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY - 1, 'whitePawn', position) ) {
					    return true
					}
			}
			
			//check knight attacks
			if (isWhiteAttacked) {
				if (isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY + 2, 'blackKnight', position) ||
				    isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY - 2, 'blackKnight', position) ||
					isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY + 2, 'blackKnight', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY - 2, 'blackKnight', position) ||
					isAttackedByStepAttacker(attackedSquareX + 2, attackedSquareY + 1, 'blackKnight', position) ||
				    isAttackedByStepAttacker(attackedSquareX + 2, attackedSquareY - 1, 'blackKnight', position) ||
					isAttackedByStepAttacker(attackedSquareX - 2, attackedSquareY + 1, 'blackKnight', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 2, attackedSquareY - 1, 'blackKnight', position) ) {
					    return true
				}
			} else {
				if (isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY + 2, 'whiteKnight', position) ||
				    isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY - 2, 'whiteKnight', position) ||
					isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY + 2, 'whiteKnight', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY - 2, 'whiteKnight', position) ||
					isAttackedByStepAttacker(attackedSquareX + 2, attackedSquareY + 1, 'whiteKnight', position) ||
				    isAttackedByStepAttacker(attackedSquareX + 2, attackedSquareY - 1, 'whiteKnight', position) ||
					isAttackedByStepAttacker(attackedSquareX - 2, attackedSquareY + 1, 'whiteKnight', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 2, attackedSquareY - 1, 'whiteKnight', position) ) {
					    return true
				}
			}
			
			//check king attacks
			if (isWhiteAttacked) {
				if (isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY, 'blackKing', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY, 'blackKing', position) ||
					isAttackedByStepAttacker(attackedSquareX, attackedSquareY + 1, 'blackKing', position) ||
				    isAttackedByStepAttacker(attackedSquareX, attackedSquareY - 1, 'blackKing', position) ||
					isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY + 1, 'blackKing', position) ||
				    isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY - 1, 'blackKing', position) ||
					isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY + 1, 'blackKing', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY - 1, 'blackKing', position) ) {
					    return true
				}
			} else {
				if (isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY, 'whiteKing', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY, 'whiteKing', position) ||
					isAttackedByStepAttacker(attackedSquareX, attackedSquareY + 1, 'whiteKing', position) ||
				    isAttackedByStepAttacker(attackedSquareX, attackedSquareY - 1, 'whiteKing', position) ||
					isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY + 1, 'whiteKing', position) ||
				    isAttackedByStepAttacker(attackedSquareX + 1, attackedSquareY - 1, 'whiteKing', position) ||
					isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY + 1, 'whiteKing', position) ||
				    isAttackedByStepAttacker(attackedSquareX - 1, attackedSquareY - 1, 'whiteKing', position) ) {
					    return true
				}
			}
			
			return isAttackedByAnySlidingPiece(isWhiteAttacked, attackedSquareX, attackedSquareY, position)
		}
		
		function isAttackedByStepAttacker(attackerSquareX, attackerSquareY, attackerPiece, position) {
			if (!isValidSquare(attackerSquareX, attackerSquareY)) return false
			if (position[COLUMNS[attackerSquareX - 1] + attackerSquareY] === attackerPiece) return true
			
			return false
		}
		
		
		function isAttackedByAnySlidingPiece(isWhiteAttacked, attackedSquareX, attackedSquareY, position) {
			if (isWhiteAttacked) {
				return isAttackedBySlidingattackerPiece(1, 0, 'blackRook', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(-1, 0, 'blackRook', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(0, 1, 'blackRook', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(0, -1, 'blackRook', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(1, 0, 'blackQueen', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(-1, 0, 'blackQueen', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(0, 1, 'blackQueen', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(0, -1, 'blackQueen', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(1, 1, 'blackBishop', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(1, -1, 'blackBishop', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(-1, 1, 'blackBishop', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(-1, -1, 'blackBishop', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(1, 1, 'blackQueen', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(1, -1, 'blackQueen', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(-1, 1, 'blackQueen', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(-1, -1, 'blackQueen', attackedSquareX, attackedSquareY, position) 
			}
		    else {
				return isAttackedBySlidingattackerPiece(1, 0, 'whiteRook', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(-1, 0, 'whiteRook', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(0, 1, 'whiteRook', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(0, -1, 'whiteRook', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(1, 0, 'whiteQueen', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(-1, 0, 'whiteQueen', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(0, 1, 'whiteQueen', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(0, -1, 'whiteQueen', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(1, 1, 'whiteBishop', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(1, -1, 'whiteBishop', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(-1, 1, 'whiteBishop', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(-1, -1, 'whiteBishop', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(1, 1, 'whiteQueen', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(1, -1, 'whiteQueen', attackedSquareX, attackedSquareY, position) ||
					   isAttackedBySlidingattackerPiece(-1, 1, 'whiteQueen', attackedSquareX, attackedSquareY, position) ||
				       isAttackedBySlidingattackerPiece(-1, -1, 'whiteQueen', attackedSquareX, attackedSquareY, position)
			}
		}
		
		function isAttackedBySlidingattackerPiece(row, col, attackerPiece, attackedSquareX, attackedSquareY, position) {
		    var nextAttackedSquareX = attackedSquareX + col
			var nextAttackedSquareY = attackedSquareY + row
			
			while (isValidSquare(nextAttackedSquareX, nextAttackedSquareY)) {
				var checkedattackerPiece = position[COLUMNS[nextAttackedSquareX - 1] + nextAttackedSquareY]
				
				if (checkedattackerPiece === attackerPiece) {
					return true
				}
				
				if (checkedattackerPiece !== undefined) {
					return false
				}
				
				nextAttackedSquareX = nextAttackedSquareX + col
			    nextAttackedSquareY = nextAttackedSquareY + row
			}
			
			return false
		}
		
		function isNeighbourSquare(square1X, square1Y, square2X, square2Y) {
			return Math.abs(square1X - square2X) <= 1 && Math.abs(square1Y - square2Y) <= 1
		}
		
		function isValidSquare(squareX, squareY) {
			return squareX <= 8 && squareX >= 1 && squareY <= 8 && squareY >= 1
		}
		
		function isChecked(piece, toPiece, fromSquare, toSquare) {
			var isWhiteChecked = piece.includes('white')
			var kingPosition = ''
			
			var newPosition = deepCopy(currentPosition)
			newPosition[toSquare] = currentPosition[fromSquare]
			delete newPosition[fromSquare]
			
			for (var i in currentPosition)
			{
                if (isWhiteChecked && newPosition[i] === 'whiteKing') {
				    kingPosition = i
					break
				}

                if (!isWhiteChecked && newPosition[i] === 'blackKing') {
				    kingPosition = i
					break
				}				
            }
			
			var square1Array = kingPosition.split('')
		    var square1X = COLUMNS.indexOf(square1Array[0]) + 1
		    var square1Y = parseInt(square1Array[1], 10)
			
			return isAttackedByAnyPiece(isWhiteChecked, square1X, square1Y, newPosition)
		}
		
		function validBishopStep(square1X, square1Y, square2X, square2Y, isWhiteStep, toPiece) {
		    if (Math.abs(square1X - square2X) !== Math.abs(square1Y - square2Y)) return false 
			
			if (square1X < square2X && square1Y < square2Y) {
			   return !hasInnerPiece(1, 1, square1X, square1Y, square2X, square2Y) 
			}				
			
			if (square1X < square2X && square1Y > square2Y) {
			   return !hasInnerPiece(-1, 1, square1X, square1Y, square2X, square2Y) 
			}				
			
			if (square1X > square2X && square1Y < square2Y) {
			   return !hasInnerPiece(1, -1, square1X, square1Y, square2X, square2Y) 
			}				
			
			if (square1X > square2X && square1Y > square2Y) {
			   return !hasInnerPiece(-1, -1, square1X, square1Y, square2X, square2Y) 
			}

            return false			
		}
 
        function validRookStep(square1X, square1Y, square2X, square2Y, isWhiteStep, toPiece) {
		    if (square1X < square2X && square1Y === square2Y) {
			   return !hasInnerPiece(0, 1, square1X, square1Y, square2X, square2Y) 
			}				
			
			if (square1X > square2X && square1Y === square2Y) {
			   return !hasInnerPiece(0, -1, square1X, square1Y, square2X, square2Y) 
			}				
			
			if (square1X === square2X && square1Y < square2Y) {
			   return !hasInnerPiece(1, 0, square1X, square1Y, square2X, square2Y) 
			}				
			
			if (square1X === square2X && square1Y > square2Y) {
			   return !hasInnerPiece(-1, 0, square1X, square1Y, square2X, square2Y) 
			}

            return false			
		}
		
		function validQueenStep(square1X, square1Y, square2X, square2Y, isWhiteStep, toPiece) {
            return validBishopStep(square1X, square1Y, square2X, square2Y, isWhiteStep, toPiece) ||
			       validRookStep(square1X, square1Y, square2X, square2Y, isWhiteStep, toPiece)
		}
		
		function validKnightStep(square1X, square1Y, square2X, square2Y, isWhiteStep, toPiece) {
			if ((Math.abs(square1X - square2X) === 1 && Math.abs(square1Y - square2Y) === 2) ||
     			(Math.abs(square1X - square2X) === 2 && Math.abs(square1Y - square2Y) === 1)){
                return true				
			}
            return false
		}
		
		function validKingStep(square1X, square1Y, square2X, square2Y, isWhiteStep, toPiece) {
            if ((Math.abs(square1X - square2X) === 1 && Math.abs(square1Y - square2Y) === 1) ||
     			(Math.abs(square1X - square2X) === 1 && Math.abs(square1Y - square2Y) === 0) ||
				(Math.abs(square1X - square2X) === 0 && Math.abs(square1Y - square2Y) === 1)){
                return true				
			}
			
			// sancok
			if (isWhiteStep) {
				if (castleAbility.includes('K') && square1X === 5 && square1Y === 1 && square2X === 7 && square2Y === 1) {
				    if (currentPosition[COLUMNS[7] + 1] === 'whiteRook') {
						return !isAttackedByAnyPiece(true, 5, 1, currentPosition) && 
                               !isAttackedByAnyPiece(true, 6, 1, currentPosition) &&
                               !isAttackedByAnyPiece(true, 7, 1, currentPosition)							   
					}						
				}
				
				if (castleAbility.includes('Q') && square1X === 5 && square1Y === 1 && square2X === 3 && square2Y === 1) {
				    if (currentPosition[COLUMNS[0] + 1] === 'whiteRook') {
						return !isAttackedByAnyPiece(true, 5, 1, currentPosition) && 
                               !isAttackedByAnyPiece(true, 4, 1, currentPosition) &&
                               !isAttackedByAnyPiece(true, 3, 1, currentPosition) &&
							   !isAttackedByAnyPiece(true, 2, 1, currentPosition)
					}						
				}
				
			}
			else {
				if (castleAbility.includes('k') && square1X === 5 && square1Y === 8 && square2X === 7 && square2Y === 8) {
				    if (currentPosition[COLUMNS[7] + 8] === 'blackRook') {
						return !isAttackedByAnyPiece(false, 5, 8, currentPosition) && 
                               !isAttackedByAnyPiece(false, 6, 8, currentPosition) &&
                               !isAttackedByAnyPiece(false, 7, 8, currentPosition)
					}						
				}
				
				if (castleAbility.includes('q') && square1X === 5 && square1Y === 8 && square2X === 3 && square2Y === 8) {
				    if (currentPosition[COLUMNS[0] + 8] === 'blackRook') {
						return !isAttackedByAnyPiece(false, 5, 8, currentPosition) && 
                               !isAttackedByAnyPiece(false, 4, 8, currentPosition) &&
                               !isAttackedByAnyPiece(false, 3, 8, currentPosition) &&
							   !isAttackedByAnyPiece(false, 2, 8, currentPosition)
					}						
				}
			}
			
			return false
		}
		
        function validStep(piece, toPiece, square1X, square2X, square1Y, square2Y, fromSquare, toSquare) {			
		    return !hasSameColor(piece, toPiece) && 
			       !isChecked(piece, toPiece, fromSquare, toSquare) &&
			       (toPiece === undefined || !toPiece.includes("King")) && 
			       ((piece.includes("Pawn") && validPawnStep(square1X, square1Y, square2X, square2Y, piece.includes("white"), toPiece)) ||
                    (piece.includes("Bishop") && validBishopStep(square1X, square1Y, square2X, square2Y, piece.includes("white"), toPiece))	||
					(piece.includes("Rook") && validRookStep(square1X, square1Y, square2X, square2Y, piece.includes("white"), toPiece)) ||
                    (piece.includes("Queen") && validQueenStep(square1X, square1Y, square2X, square2Y, piece.includes("white"), toPiece)) ||
					(piece.includes("Knight") && validKnightStep(square1X, square1Y, square2X, square2Y, piece.includes("white"), toPiece)) ||
                    (piece.includes("King") && validKingStep(square1X, square1Y, square2X, square2Y, piece.includes("white"), toPiece)))
 		}

        function changeNextColor() {
		    if(nextColor.includes("white")) {
			    nextColor = "black"
			}
			else {
			    nextColor = "white"
			}
		}			
		
		function buildPieceHtml(piece, isHidden, id) {
			var html = '<img src="img/' + piece + '.png" '
			html += 'id="' + id + '" '
			html += 'alt="" class="' + CSS.piece + '" data-piece="' + piece + '" style="width:' + squareSize + 'px;height:' + squareSize + 'px;' 
			
			if (isHidden)
			{
				html += 'display:none;'
			}
			
			html += '" />'
			
			return html;
		}
		
		function stopDefault(evt) {
		    evt.preventDefault()
		}
		
		// capture the x, y coordinates of all squares in memory
		function captureSquareOffsets() {
		    squareOffsets = {}
			
			for (var i in squareIds) {
			    if (!squareIds.hasOwnProperty(i)) continue
				
				squareOffsets[i] = $('#' + squareIds[i]).offset()
			}
		}
		
		function beginDraggingPiece(source, piece, x, y) {
		    // set state
            isDragging = true
            draggedPiece = piece
            draggedPieceSource = source
            draggedPieceLocation = source

 			// capture the x, y coordinates of all squares in memory
			captureSquareOffsets()
			
			
			// create the dragged piece
			$draggedPiece.attr('src', 'img/' + piece + '.png').css({
		        display: '',
				position: 'absolute',
				left: x - squareSize/2,
				top: y - squareSize/2
			  })

			//highlight the source square and hide the piece
			$('#' + squareIds[source])
			    .addClass(CSS.highlight1)
				.find('.' + CSS.piece)
				.css('display', 'none')
		}
		 
		function mouseenterSquare(evt) {
		    // do not fire this event if we are  dragging a piece 
			// this should never happen, but it's a safeguard
			if (isDragging) return
		}

		function mouseleaveSquare(evt) {
		    // do not fire this event if we are  dragging a piece 
			// this should never happen, but it's a safeguard
			if (isDragging) return
		}
		
		function mousedownSquare(evt) {
			// do nothing if there is no piece on this square
			var square = $(this).attr('data-square')
			if (!validSquare(square)) return
			if (!validSide(currentPosition[square])) return
			if (!currentPosition.hasOwnProperty(square)) return
			
			beginDraggingPiece(square, currentPosition[square], evt.pageX, evt.pageY)
		} 
		
		function getXYOnSquare(x, y) {
		    for (var i in squareOffsets) {
			    if (!squareOffsets.hasOwnProperty(i)) continue
				
				var square = squareOffsets[i]
				
				if (x >= square.left &&
				    x < square.left + squareSize &&
					y >= square.top &&
				    y < square.top + squareSize) {
			        return i
				}
			}
			
			return 'offboard'
		}
		
		function removeSquareHighlights() {
			$board
			    .find('.' + CSS.square)
				.removeClass(CSS.highlight1 + ' ' + CSS.highlight2)
		}
		
		function trashDraggedPiece() {
            removeSquareHighlights()
			
			// remove the source piece
			var newPosition = deepCopy(currentPosition)
            delete newPosition[draggedPieceSource]
            setCurrentPosition(newPosition)
			
			// redraw postion
			drawPositionInstant()
			
			// hide the dragged piece
			$draggedPiece.fadeOut(4000)
			
			// set state
			isDragging = false
		}

        function trashDraggedPieceImmediately() {
            removeSquareHighlights()
			
			// hide the dragged piece
			$draggedPiece.fadeOut(0)
			
			// set state
			isDragging = false
		}
		
		function dropDraggedPieceOnSquare(square, newPosition) {
			removeSquareHighlights()
			
			if (newPosition === undefined) {
				newPosition = deepCopy(currentPosition)
			}
			
			delete newPosition[draggedPieceSource]
			newPosition[square] = draggedPiece
			setCurrentPosition(newPosition)
			
			// get target square information
			var targetSquarePosition = $('#' + squareIds[square]).offset()
			
			// animation complete
			function onAnimationCompleted() {
				drawPositionInstant()
				$draggedPiece.css('display', 'none')
			}
			
			// snap the piece to the target square 
			var options = {
				duration: 30,
                complete: onAnimationCompleted		
			}
			
			$draggedPiece.animate(targetSquarePosition, options)
			
			// set state
			isDragging = false
		}
		
		function resetDraggedPieceSquare() {
			removeSquareHighlights()
			
			// get target square information
			//var targetSquarePosition = $('#' + squareIds[square]).offset()
			
			// animation complete
			function onAnimationCompleted() {
				drawPositionInstant()
				$draggedPiece.css('display', 'none')
			}
			
			animateDraggedPieceToSquare(draggedPieceSource, onAnimationCompleted)
			
			// set state
			isDragging = false
		}

        String.prototype.replaceAt = function(index, replacement) {
			var t1 = this.substr(0, index)
			var t2 = this.substr(index + replacement.length)
            return this.substr(0, index) + replacement + this.substr(index + replacement.length);
        }
        
		function removeFromCastleString(oldCastleString, castledSide)
		{
		    var castlingPieces = oldCastleString.split('')
		    var newCastlingPieces = []
			
			for (i = 0; i < castlingPieces.length; i++) {
                if (castlingPieces[i] !== castledSide) {
					newCastlingPieces.push(castlingPieces[i])
				}
            }
			
			return newCastlingPieces.join('')
		}

        function saveFen(location, piece, square1X, square2X, square1Y, square2Y) {
			var newPosition = deepCopy(positions[stepCounter])
			
		    var stepDigitCount = stepCounter > 9 ? (stepCounter > 99 ? 3 : 2) : 1; 
			var newPositionSubString = newPosition.substr(newPosition, newPosition.length - stepDigitCount)
			
            newPositionSubString = expandFenEmptySquares(newPositionSubString)	

           	var fromIndex = (8 - square1Y) * 9 + square1X - 1  // 9 since separators: '/' is added 
			var toIndex = (8 - square2Y) * 9 + square2X - 1
			
		    var fromFenPiece = newPositionSubString.charAt(fromIndex)
			
			if ( fromFenPiece === 'P' && piece === 'whiteQueen') {
				fromFenPiece = 'Q'
			}
			
			if ( fromFenPiece === 'p' && piece === 'blackQueen') {
				fromFenPiece = 'q'
			}
			
			newPositionSubString = newPositionSubString.replaceAt(toIndex, fromFenPiece)
			newPositionSubString = newPositionSubString.replaceAt(fromIndex, "1")
			
			if (piece.includes('whiteKing') && square1X === 5 && square2X === 7) { // white king side castling
			    newPositionSubString = newPositionSubString.replaceAt(9 * 7 + 5, "R") // 9 since operator '/'
			    newPositionSubString = newPositionSubString.replaceAt(9 * 7 + 7, "1") 
			}				

            if (piece.includes('whiteKing') && square1X === 5 && square2X === 3) { // white queen side castling
			    newPositionSubString = newPositionSubString.replaceAt(9 * 7 + 3, fromFenPiece) // 9 since operator '/'
			    newPositionSubString = newPositionSubString.replaceAt(9 * 7, "R")
			}

            if (piece.includes('blackKing') && square1X === 5 && square2X === 7) { // black king side castling
			    newPositionSubString = newPositionSubString.replaceAt(5, "r") // 9 since operator '/'
			    newPositionSubString = newPositionSubString.replaceAt(7, "1")
			}

            if (piece.includes('blackKing') && square1X === 5 && square2X === 3) { // black queen side castling
			    newPositionSubString = newPositionSubString.replaceAt(3, fromFenPiece) // 9 since operator '/'
			    newPositionSubString = newPositionSubString.replaceAt(0, "1")
			}
			
			
			newPositionSubString = newPositionSubString.replaceAt(72, newPositionSubString.charAt(72) === "w" ? "b" : "w")
			
			stepCounter++
			
			newPositionSubString = squeezeFenEmptySquares(newPositionSubString)
						
			newPosition =  newPositionSubString + stepCounter
			
			var newPositionPieces = newPosition.split(' ')
			
			if (piece.includes('Pawn') && Math.abs(square1Y - square2Y) == 2) {
				var enpassantSquareY = ''
				if (piece.includes('white')) {
			        enpassantSquareY = square1Y + 1
				} else {
					enpassantSquareY = square1Y - 1
				}
				newPositionPieces[3] = COLUMNS[square1X -1] + enpassantSquareY
			} else {
				newPositionPieces[3] = '-'
			}
			
			if (piece.includes('whiteKing') && square1X === 5 && square2X === 7) { // white king side castling
			    newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'K')
			}				

            if (piece.includes('whiteKing') && square1X === 5 && square2X === 3) { // white queen side castling
			    newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'Q')
			}

            if (piece.includes('blackKing') && square1X === 5 && square2X === 7) { // black king side castling
			    newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'k')
			}

            if (piece.includes('blackKing') && square1X === 5 && square2X === 3) { // black queen side castling
			    newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'q')
			}
			
			if (piece.includes('whiteKing')) {
				newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'K')
				newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'Q')
			}	
			
			if (piece.includes('blackKing')) {
				newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'k')
				newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'q')
			}
			
			if (piece.includes('whiteRook')) {
				if (square1X === 8 && square1Y === 1) {
				   newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'K')
				}
				
                if (square1X === 1 && square1Y === 1) {
				   newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'Q')
				}
			}
			
			if (piece.includes('blackRook')) {
				if (square1X === 8 && square1Y === 8) {
				   newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'k')
				}
				
                if (square1X === 1 && square1Y === 8) {
				   newPositionPieces[2] = removeFromCastleString(newPositionPieces[2], 'q')
				}
			}
			
			if (newPositionPieces[2] === '')
			{
				newPositionPieces[2] = '-'
			}
			
			castleAbility = newPositionPieces[2]
			enpassantTarget = newPositionPieces[3]
			
			newPosition = newPositionPieces.join(' ')
			
			positions.push(newPosition)
			
			var fromSquare = COLUMNS[square1X-1] + square1Y
			
			if (stepCounter & 1) {
			    $('#stepTable > tbody:last-child').append('<tr><td><button class="buttonTable button1" id="step' + stepCounter + '">' + stepCounter + ' ' + fromSquare + location + '</button></tr>');
			}
			else {
			    $('#stepTable2 > tbody:last-child').append('<tr><td><button class="buttonTable button1" id="step' + stepCounter + '">' + stepCounter + ' ' + fromSquare + location + '</button></tr>');
			}
			$('#step' + stepCounter).on('click', stepClicked)
			$('#fenId').val(newPosition)
		}			
		
		function addCastling(draggedPiece, square1X, square2X, square1Y, square2Y) {
			var newPosition = deepCopy(currentPosition)
			
			if (draggedPiece === 'whiteKing' && square1X === 5 && square2X === 7) {
				castlingToObject(0, newPosition) 
			}
			
			if (draggedPiece === 'whiteKing' && square1X === 5 && square2X === 3) {
				castlingToObject(1, newPosition) 
			}
			
			if (draggedPiece === 'blackKing' && square1X === 5 && square2X === 7) {
				castlingToObject(2, newPosition) 
			}
			
			if (draggedPiece === 'blackKing' && square1X === 5 && square2X === 3) {
				castlingToObject(3, newPosition) 
			}
			
			return newPosition
		}
		
		function stopDraggedPiece(location) {
			var square1Array = draggedPieceSource.split('')
		    var square1X = COLUMNS.indexOf(square1Array[0]) + 1
		    var square1Y = parseInt(square1Array[1], 10)
			var square2Array = location.split('')
		    var square2X = COLUMNS.indexOf(square2Array[0]) + 1
		    var square2Y = parseInt(square2Array[1], 10)
			
			if(!validStep(draggedPiece, currentPosition[location], square1X, square2X, square1Y, square2Y, draggedPieceSource, location)) {
			    resetDraggedPieceSquare()
				return
			}				
			
			if (location === 'offboard') {
			    // trashDraggedPiece()
				resetDraggedPieceSquare()
			} else {
				if (draggedPiece === 'whitePawn' && square2Y === 8) {
					draggedPiece = 'whiteQueen';
				}
				
				if (draggedPiece === 'blackPawn' && square2Y === 1) {
					draggedPiece = 'blackQueen';
				}
				
				saveFen(location, draggedPiece, square1X, square2X, square1Y, square2Y)
				var toPiece = currentPosition[COLUMNS[square2X - 1] + square2Y]
				
				// check enpassant
				if (draggedPiece.includes('Pawn') && toPiece === undefined && Math.abs(square1X - square2X) === 1 && Math.abs(square1Y - square2Y) === 1) {
					delete currentPosition[COLUMNS[square2X - 1] + square1Y]
				}
				
				trashDraggedPieceImmediately()
				var newPosition = addCastling(draggedPiece, square1X, square2X, square1Y, square2Y)
				dropDraggedPieceOnSquare(location, newPosition)
				changeNextColor()
				
				var inputFen = $('#fenId').val()
		        worker.postMessage(inputFen);
			}
		}
		
		function mouseupWindow(evt) {
			// do nothing if we aren't dragging a piece
			if (!isDragging) return
			
			// get the location
			var location = getXYOnSquare(evt.pageX, evt.pageY)
			
			stopDraggedPiece(location)
		}
		
		function mousemoveWindow(evt) {
			if (isDragging) {
				// update dragged piece
	            var x = evt.pageX
                var y = evt.pageY

                // put the dragged piece over the cursor
                $draggedPiece.css({
				    left: x - squareSize / 2,
					top: y - squareSize / 2
				})			

                // get location 
                var location = getXYOnSquare(x, y)

                // do nothing if the location hasn't changed
                if (location === draggedPieceLocation) return
 
                // remove heghlight from previous square
                if (validSquare(draggedPieceLocation)) {
				    $('#' + squareIds[draggedPieceLocation]).removeClass(CSS.highlight2)
				}	

                // add highlight to new square
                if (validSquare(location)) {
				    $('#' + squareIds[location]).addClass(CSS.highlight2)
				}	
               
                // update state
                draggedPieceLocation = location				
			}
		}
		
		var throttledMousemoveWindow = throttle(mousemoveWindow)
		
	    function addEvents() {
			// prevent "image drag"
			$('body').on('mousedown mousemove', '.' + CSS.piece, stopDefault)
			
			// mouse drag pieces
			$board.on('mousedown', '.' + CSS.square, mousedownSquare)
			
			// mouse enter / leave square
			$board
                .on('mouseenter', '.' + CSS.square, mouseenterSquare)
                .on('mouseleave', '.' + CSS.square, mouseleaveSquare)
			
			// piece drag
			$window = $(window)
			$window
			    .on('mousemove', throttledMousemoveWindow)
				.on('mouseup', mouseupWindow)			
		}
		
		// search square size based on the width of container
        // search mod 8 with reducing one trick (see below)		
		function calculateSquareSize() {
		    var containerWidth = parseInt($container.width(), 10)
			var borderWidth = containerWidth - 1
			
			while (borderWidth % 8 !== 0 && borderWidth > 0) {
				borderWidth--
			}				
			
			return borderWidth / 8
		}

        function drawPositionInstant() {
		    // clear board
			$board.find('.' + CSS.piece).remove()
			
			// add the pieces
			for (var i in currentPosition)
			{
				if(!currentPosition.hasOwnProperty(i)) continue 
				
				$('#' + squareIds[i]).append(buildPieceHtml(currentPosition[i], false, i))
			}
		}

		function drawBoard() {
			
			// build Board Html
			var html = ''
			
			// algebratic notation
			var alpha = deepCopy(COLUMNS)
			var row = 8
			var squareColor = 'black'
			
			for( var i = 0; i < 8; i++) {
				
				html += '<div class="' + CSS.row + '">'
				
				for( var j = 0; j < 8; j++) {
	                var square = alpha[j] + row
					
					html += '<div class="' + CSS.square + ' ' + CSS[squareColor] + ' ' + 
					    'square-' + square + '" ' + 
						'style="width:' + squareSize + 'px;height:' + squareSize + 'px;" ' +
                        'id="' + squareIds[square] + '" ' +
                        'data-square="' + square + '">'						
					
					// alpha notation
					if (row === 1)
					{
						html += '<div class="' + CSS.notation + ' ' + CSS.alpha + '">' + alpha[j] + '</div>'
					}
					
					// numeric notation
					if (j === 0)
					{
						html += '<div class="' + CSS.notation + ' ' + CSS.numeric + '">' + row + '</div>'
					}
				
				    html += '</div>' // end square
					
					squareColor = (squareColor === 'white') ? 'black' : 'white' 
				}
				
				html += '<div class="' + CSS.clearfix + '"></div></div>'
				
				squareColor = (squareColor === 'white') ? 'black' : 'white'
					
			    row--;
			}

            $board.html(html)
			
			drawPositionInstant()
		}
		
		widget.resize = function() {
			// calculate new square size
			squareSize = calculateSquareSize()
			
			// set board width
			$board.css('width', squareSize * 8 + 'px')
			
			// set dragged piece size
			$draggedPiece.css({width: squareSize, height: squareSize})
			
			// now it's time to draw the whole board
			drawBoard()
		}
		
		function initDOM() {
			// create unique square ids
			for (var i = 0; i < COLUMNS.length; i++) {
				for (var j = 1; j <= 8; j++) {
					var square = COLUMNS[i] + j
					squareIds[square] = square + '-' + generateUniqueId()
				}
			}
			
			$container.html('<div class="' + CSS.chessboard + '"><div class="' + CSS.board + '"></div></div>') 
			$board = $container.find('.' + CSS.board)
			var draggedPieceId = generateUniqueId()
			$('body').append(buildPieceHtml('whitePawn', true, draggedPieceId))
			$draggedPiece = $('#' + draggedPieceId)
			
			// get border size
			boardBorderSize = parseInt($board.css('borderLeftWidth'), 10)
            
			// resize and draw
            widget.resize()					    	
		}
		
		initDOM()
		addEvents()
		
		return widget
	}
 
    // exposing part
    window['Chessboard'] = constructor
	window['ChessBoard'] = window['Chessboard']
	
	
})() 
