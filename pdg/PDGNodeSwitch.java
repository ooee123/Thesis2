package pdg;

import ast.*;
import com.google.common.collect.Lists;
import com.google.common.collect.SetMultimap;
import lombok.EqualsAndHashCode;
import lombok.NonNull;
import lombok.Value;

import java.util.*;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeSwitch extends PDGNodeContainsStatementNode<SelectionStatementSwitch> {
    @NonNull private List<Collection<PDGNode<? extends BlockItem>>> cases;
    private Collection<PDGNode<? extends BlockItem>> defaultCase;

    public PDGNodeSwitch(SelectionStatementSwitch selectionStatementSwitch, List<Collection<PDGNode<? extends BlockItem>>> cases) {
        this(selectionStatementSwitch, cases, null, false);
    }

    public PDGNodeSwitch(SelectionStatementSwitch selectionStatementSwitch, List<Collection<PDGNode<? extends BlockItem>>> cases, Collection<PDGNode<? extends BlockItem>> defaultCase, boolean required) {
        super(selectionStatementSwitch, required);
        this.cases = cases;
        this.defaultCase = defaultCase;
    }

    @Override
    public SelectionStatementSwitch sort(PDGSorter sorter) {
        List<BlockItem> blockItems = new ArrayList<>();
        for (Collection<PDGNode<? extends BlockItem>> aCase : cases) {
            blockItems.addAll(sorter.sort(aCase).getBlockItems());
        }
        if (this.defaultCase != null) {
            blockItems.addAll(sorter.sort(this.defaultCase).getBlockItems());
        }
        return new SelectionStatementSwitch(blockItem.getExpression(), new CompoundStatement(blockItems));
    }

    @Override
    public Collection<PDGNode<? extends Statement>> getStatementNodes() {
        Collection<PDGNode<? extends Statement>> statements = new ArrayList<>();
        for (Collection<PDGNode<? extends BlockItem>> aCase : cases) {
            for (PDGNode<? extends BlockItem> pdgNode : aCase) {
                if (pdgNode.blockItem instanceof Statement) {
                    statements.add((PDGNode<? extends Statement>) pdgNode);
                }
            }
        }
        for (PDGNode<? extends BlockItem> pdgNode : defaultCase) {
            if (pdgNode.blockItem instanceof Statement) {
                statements.add((PDGNode<? extends Statement>) pdgNode);
            }
        }
        return statements;
        /*
        Collection<PDGNode<? extends Statement>> statement = new ArrayList<>();
        statement.add(new PDGNodeCompoundStatement())
        return new ArrayList<>();
        */
    }
/*
    public void removeEmptyCompoundStatement() {
        if (elseNode instanceof PDGNodeCompoundStatement) {
            if (((PDGNodeCompoundStatement) elseNode).getBody().isEmpty()) {
                elseNode = null;
            }
        }
    }
    */
}
