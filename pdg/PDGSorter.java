package pdg;

import ast.BlockItem;
import ast.statement.impl.CompoundStatement;

import java.util.Collection;

/**
 * Created by ooee on 10/14/16.
 */
public interface PDGSorter {

    /**
     * Takes a PDG represented by a collection of PDGNode and converts it into a linear list of BlockItems.
     * It is assumed that all PDGNode in nodes are in the same level
     * @param nodes
     * @return
     */
    public CompoundStatement sort(Collection<PDGNode<? extends BlockItem>> nodes);
}
