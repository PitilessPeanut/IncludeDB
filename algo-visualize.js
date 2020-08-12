// Copyright (c) 2020 Professor Peanut
// MIT license see accompanying file 'nanopulsedb.h' for details

// import visualization libraries {
const { Tracer, Array1DTracer, Array2DTracer, LogTracer, Layout, VerticalLayout } = require('algorithm-visualizer');
// }

var ni = [  0,  0,  0,  0,  0,  0,  0];
var nf = [  0,  0,  0,  0,  0,  0,  0];
var nn = [  [0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0] ];
const N = ni.length;

// define tracer variables {
const tracerids = new Array1DTracer('ids');
const tracerfile = new Array1DTracer('filepos');
const tracernext = new Array2DTracer('next');
const logger = new LogTracer('Console');
Layout.setRoot(new VerticalLayout([tracerids, tracerfile, tracernext, logger]));
tracerids.set(ni);
tracerfile.set(nf);
tracernext.set(nn);
Tracer.delay();
// }

var headD = 0;
var nKeys = 0;

function getNewNodePos() {
    return nKeys++;
}

function insertSkipnode(key, filepos) {
    const newNodeAddr = getNewNodePos();
    // visualize {
    logger.println(`new node addr: ${newNodeAddr}`);
    logger.println(`nKeys: ${nKeys}`);
    tracerids.select(newNodeAddr);
    tracerfile.select(newNodeAddr);
    tracernext.select(3, newNodeAddr)
    Tracer.delay();
    // }
    ni[newNodeAddr] = key;
    nf[newNodeAddr] = filepos;
    // visualize {
    tracerids.patch(newNodeAddr, ni[newNodeAddr]);
    tracerfile.patch(newNodeAddr, nf[newNodeAddr]);
    Tracer.delay();
    tracerids.depatch(newNodeAddr);
    tracerfile.depatch(newNodeAddr);
    // }
    
    if (key < ni[headD]) {
        nn[newNodeAddr] = headD;
        // visualize {
        tracernext.patch(3, newNodeAddr, headD);
        Tracer.delay();
        tracernext.depatch(3, newNodeAddr);
        // }
        headD = newNodeAddr;
        return;
    }
    
    var current = headD;
    var next = 0;
    
    for (var i=0; i<nKeys-2; i++) {
        next = nn[current];
        // visualize {
        logger.println(`  current: ${current}`);
        logger.println(`  next: ${next}`);
        tracerids.deselect(0, N-1);
        tracerfile.deselect(0, N-1);
        tracernext.deselect(0, 0, 3, 6);
        tracerids.select(current);
        tracerfile.select(current);
        tracernext.select(3, current);
        Tracer.delay();
        // }
        if (ni[next] > key) {
            // visualize {
            logger.println(`--- break ----`);
            Tracer.delay();
            // }
            nn[newNodeAddr] = next;
            // visualize {
            tracernext.patch(3, newNodeAddr, next);
            Tracer.delay();
            tracernext.depatch(3, newNodeAddr);
    // }
            break;
        }
        current = next;
    }
    
    // visualize {
    logger.println(`  current: ${current}`);
    Tracer.delay();
    // }
    

    
    
    nn[current] = newNodeAddr;
    // visualize {
    tracernext.patch(3, current, newNodeAddr);
    Tracer.delay();
    tracernext.depatch(3, current);
    // }
}

function start() {
    insertSkipnode(500, 0);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(111, 1);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(444, 2);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(222, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(333, 4);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(44, 5);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(555, 6);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
}

start();


