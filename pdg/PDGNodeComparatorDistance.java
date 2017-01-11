package pdg;

import ast.BlockItem;

import java.util.Comparator;

/**
 * Created by ooee on 10/27/16.
 */
public class PDGNodeComparatorDistance implements Comparator<PDGNode<? extends BlockItem>> {
    @Override
    public int compare(PDGNode<? extends BlockItem> pdgNode1, PDGNode<? extends BlockItem> pdgNode2) {
        return Integer.compare(pdgNode1.getIsADependencyFor().size(), pdgNode2.getIsADependencyFor().size());
    }
}
