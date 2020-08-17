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
var vv = [  0,  0,  0,  0,  0,  0,  0];
const N = ni.length;

// define tracer variables {
const tracerids = new Array1DTracer('ids');
const tracerfile = new Array1DTracer('filepos');
const tracernext = new Array2DTracer('next');
const tracervis = new Array1DTracer('visits');
const logger = new LogTracer('Console');
Layout.setRoot(new VerticalLayout([tracerids,
                                   tracerfile,
                                   tracernext,
                                   tracervis,
                                   logger]));
tracerids.set(ni);
tracerfile.set(nf);
tracernext.set(nn);
tracervis.set(vv);
Tracer.delay();
// }

var heads = [0,0,0,0];
var visits = 0;
var nKeys = 0;//[0,0,0,0];
var nBottomLen = 0;

function getNewNodePos() {
    return nKeys++;
}

function insertNewSkipnode(key, filepos) {
    const newNodeAddr = getNewNodePos();
    
    // visualize {
    logger.println(`new node addr: ${newNodeAddr}`);
    logger.println(`nKeys: ${nKeys}`);
    tracerids.select(newNodeAddr);
    tracerfile.select(newNodeAddr);
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
    
    insertSkipnode(key, newNodeAddr);
}

function insertSkipnode(key, pos) {
    // visualize {
    tracernext.select(3, pos);
    // }
    
    if (key < ni[heads[3]]) {
        nn[0][pos] = pos;
        nn[1][pos] = pos;
        nn[2][pos] = pos;
        nn[3][pos] = heads[3];
        // visualize {
        tracernext.patch(0, pos, nn[0][pos]);
        tracernext.patch(1, pos, nn[1][pos]);
        tracernext.patch(2, pos, nn[2][pos]);
        tracernext.patch(3, pos, nn[3][pos]);
        Tracer.delay();
        tracernext.depatch(0, pos);
        tracernext.depatch(1, pos);
        tracernext.depatch(2, pos);
        tracernext.depatch(3, pos);
        // }
        heads[0] = pos;
        heads[1] = pos;
        heads[2] = pos;
        heads[3] = pos;
        return;
    }
    
    var current = heads[3];
    var next = 0;
    
    for (var i=0; i<nKeys-2; i++) {
        next = nn[3][current];
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
            nn[0][pos] = pos;
            nn[1][pos] = pos;
            nn[2][pos] = pos;
            nn[3][pos] = next;
            // visualize {
            tracernext.patch(0, pos, nn[0][pos]);
            tracernext.patch(1, pos, nn[1][pos]);
            tracernext.patch(2, pos, nn[2][pos]);
            tracernext.patch(3, pos, next);
            Tracer.delay();
            tracernext.depatch(0, pos);
            tracernext.depatch(1, pos);
            tracernext.depatch(2, pos);
            tracernext.depatch(3, pos);
            // }
            break;
        }
        current = next;
    }
    
    // visualize {
    logger.println(`  current: ${current}`);
    Tracer.delay();
    // }
    

    
    
    nn[3][current] = pos;
    // visualize {
    tracernext.patch(3, current, pos);
    Tracer.delay();
    tracernext.depatch(3, current);
    // }
}

function promoteSkipnode(key, pos, layer) {
    // visualize {
    tracernext.select(layer, pos);
    Tracer.delay();
    tracernext.deselect(0,0,3,6);
    // }
    
    var current = heads[2];
    var next = 0;
    
    for (var i=0; i<nKeys-2; i++) {
        next = nn[2][current];
    }
    
    nn[2][current] = pos;
    // visualize {
    tracernext.patch(2, current, nn[2][current]);
    Tracer.delay();
    tracernext.depatch(2, current);
    // }
}

function findSkipnode(key, start, layer) {
    var current = start;
    var prev = current;
    for (var i=0; i<nKeys; i++) {
        // visualize {
        tracerids.select(current);
        tracerfile.select(current);
        tracernext.select(layer, current);
        Tracer.delay();
        tracerids.deselect(0, N-1);
        tracerfile.deselect(0, N-1);
        tracernext.deselect(0, 0, 3, 6);
        // }
        if (layer <3 && (prev===current || ni[current] > key)) {
            // overshoot or empty
            // visualize {
            tracervis.select(current);
            logger.println(`layer: ${layer} ni[cur] ${ni[current]} prev ${prev} cur ${current}`);
            Tracer.delay();
            tracervis.deselect(0, N-1);
            // }
            return findSkipnode(key, nn[layer+1][prev], layer+1);
        } else if (ni[current] === key) {
            vv[current] += 1;
            visits += 1;
            if (((vv[current]*100) / visits) > 20) {
                // promote:
                // visualize {
                logger.println(`promote: ${layer-1} `);
                Tracer.delay();
                // }
                promoteSkipnode(key, current, layer-1);
            }
            
            // visualize {
            tracervis.patch(current, vv[current]);
            Tracer.delay();
            tracervis.depatch(current);
            // }
            return current;
        }
        prev = current;
        current = nn[layer][current];
    }
    return "not found";
}

function start() {
    insertNewSkipnode(500, 0);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(111, 1);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(444, 2);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(222, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(333, 4);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(44, 5);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(555, 6);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    // visualize {
    logger.println(` --- Searching ---`);
    // }
    const resA = findSkipnode(123, heads[0], 0);
    // visualize {
    logger.println(`resA (should be 'not found'): ${resA}`);
    Tracer.delay();
    // }
    const resB = findSkipnode(222, heads[0], 0);
    // visualize {
    logger.println(`resB (should be 3): ${resB}`);
    Tracer.delay();
    // }
    const resC = findSkipnode(333, heads[0], 0);
    // visualize {
    logger.println(`resC(should be 4): ${resC}`);
    Tracer.delay();
    // }
}

start();


