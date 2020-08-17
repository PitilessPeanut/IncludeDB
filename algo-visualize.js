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
var visits = [0,0,0,0];
var nKeys = 0;//[0,0,0,0];
var nBottomLen = 0;

function getNewNodePos() {
    return nKeys++;
}

function insertNewSkipnode(key, filepos) {
    
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
    
    if (key < ni[heads[3]]) {
        nn[0][newNodeAddr] = newNodeAddr;
        nn[1][newNodeAddr] = newNodeAddr;
        nn[2][newNodeAddr] = newNodeAddr;
        nn[3][newNodeAddr] = heads[3];
        // visualize {
        tracernext.patch(0, newNodeAddr, nn[0][newNodeAddr]);
        tracernext.patch(1, newNodeAddr, nn[1][newNodeAddr]);
        tracernext.patch(2, newNodeAddr, nn[2][newNodeAddr]);
        tracernext.patch(3, newNodeAddr, nn[3][newNodeAddr]);
        Tracer.delay();
        tracernext.depatch(0, newNodeAddr);
        tracernext.depatch(1, newNodeAddr);
        tracernext.depatch(2, newNodeAddr);
        tracernext.depatch(3, newNodeAddr);
        // }
        heads[0] = newNodeAddr;
        heads[3] = newNodeAddr;
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
            nn[0][newNodeAddr] = newNodeAddr;
            nn[1][newNodeAddr] = newNodeAddr;
            nn[2][newNodeAddr] = newNodeAddr;
            nn[3][newNodeAddr] = next;
            // visualize {
            tracernext.patch(0, newNodeAddr, nn[0][newNodeAddr]);
            tracernext.patch(1, newNodeAddr, nn[1][newNodeAddr]);
            tracernext.patch(2, newNodeAddr, nn[2][newNodeAddr]);
            tracernext.patch(3, newNodeAddr, next);
            Tracer.delay();
            tracernext.depatch(0, newNodeAddr);
            tracernext.depatch(1, newNodeAddr);
            tracernext.depatch(2, newNodeAddr);
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
    

    
    
    nn[3][current] = newNodeAddr;
    // visualize {
    tracernext.patch(3, current, newNodeAddr);
    Tracer.delay();
    tracernext.depatch(3, current);
    // }
}

function promoteSkipnode(key, layer) {
    
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
            logger.println(`layer: ${layer}`);
            Tracer.delay();
            tracervis.deselect(0, N-1);
            // }
            return findSkipnode(key, nn[layer+1][prev], layer+1);
        } else if (ni[current] === key) {
            vv[current] += 1;
            visits[layer] += 1;
            if (((vv[current]*100) / visits[layer]) > 20) {
                // promote:
                // visualize {
                logger.println(`promote: ${layer-1} `);
                Tracer.delay();
                // }
                insertSkipnode(key, /*nf[current]*/321, layer-1);
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
    const resA = findSkipnode(123, heads[0], 0);
    // visualize {
    logger.println(`resA (should be 'not found'): ${resA}`);
    // }
    const resB = findSkipnode(222, heads[0], 0);
    // visualize {
    logger.println(`resB (should be 3): ${resB}`);
    // }
}

start();


