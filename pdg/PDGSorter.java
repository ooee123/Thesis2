package pdg;

import ast.BlockItem;
import ast.Statement;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

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
    public Statement sort(Collection<PDGNode> nodes);
}
