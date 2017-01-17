package pdg;

import ast.*;
import ast.statement.Statement;
import ast.statement.impl.CompoundStatement;
import ast.statement.impl.SelectionStatementSwitch;
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
    @NonNull private List<List<PDGNode<? extends BlockItem>>> cases;
    private List<PDGNode<? extends BlockItem>> defaultCase;

    public PDGNodeSwitch(SelectionStatementSwitch selectionStatementSwitch, List<List<PDGNode<? extends BlockItem>>> cases) {
        this(selectionStatementSwitch, cases, null, false);
    }

    public PDGNodeSwitch(SelectionStatementSwitch selectionStatementSwitch, List<List<PDGNode<? extends BlockItem>>> cases, List<PDGNode<? extends BlockItem>> defaultCase, boolean required) {
        super(selectionStatementSwitch, required);
        this.cases = cases;
        this.defaultCase = defaultCase;
    }

    @Override
    public SelectionStatementSwitch sort(PDGSorter sorter) {
        List<BlockItem> blockItems = new ArrayList<>();
        for (List<PDGNode<? extends BlockItem>> aCase : cases) {
            PDGNode<? extends BlockItem> labeledBlockItem = aCase.remove(0);
            List<BlockItem> caseBlockItems = sorter.sort(aCase).getBlockItems();
            caseBlockItems.add(0, labeledBlockItem.blockItem);
            blockItems.addAll(caseBlockItems);
        }
        if (this.defaultCase != null) {
            PDGNode<? extends BlockItem> defaultBlockItem = this.defaultCase.remove(0);
            List<BlockItem> defaultBlockItems = sorter.sort(this.defaultCase).getBlockItems();
            defaultBlockItems.add(0, defaultBlockItem.blockItem);
            blockItems.addAll(defaultBlockItems);
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
        if (defaultCase != null) {
            for (PDGNode<? extends BlockItem> pdgNode : defaultCase) {
                if (pdgNode.blockItem instanceof Statement) {
                    statements.add((PDGNode<? extends Statement>) pdgNode);
                }
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
