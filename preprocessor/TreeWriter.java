import edu.stanford.nlp.ling.HasWord;
import edu.stanford.nlp.parser.lexparser.LexicalizedParser;
import edu.stanford.nlp.process.DocumentPreprocessor;
import edu.stanford.nlp.trees.Tree;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author sean
 */
public class TreeWriter
{
    private final static String INPUTPREFIX = "../../senior-thesis-data/input/";
    private final static String OUTPUTPREFIX = "../../senior-thesis-data/output/";
    
    /**
     * Preprocesses the document via the Stanford Parser's DocumentPreprocessor, finding sentence boundaries.
     * @param fileInput - The document to process
     * @return a list of sentences
     */
    private static List<List<? extends HasWord>> setupSentences(String filename)
    {
        DocumentPreprocessor dp = new DocumentPreprocessor(filename);
        List<List<? extends HasWord>> ret = new ArrayList<List<? extends HasWord>>();
        for(List<HasWord> sentence : dp)
            ret.add(sentence);
        return ret;
    }
    
    /**
     * 
     * @param filenames
     * @param maxSentenceLength 
     */
    public static void process(String[] filenames, String maxSentenceLength) throws IOException
    {
        LexicalizedParser parser = new LexicalizedParser("/home/sean/projects/author-match-data/englishPCFG.ser.gz");
        parser.setOptionFlags("-maxLength", maxSentenceLength, "-retainTmpSubcategories");
        int filesDone = 0;
        for(String filename : filenames)
        {
            FileWriter writer = new FileWriter(OUTPUTPREFIX + filename + ".tree");
            System.out.println("Parsing " + filename + " ... ");
            String numDone = "[" + (filesDone + 1) + "/" + filenames.length + "]: ";
            Iterable<List<? extends HasWord>> sentences = setupSentences(INPUTPREFIX + filename);
            double totalSentences = ((ArrayList<List<? extends HasWord>>)sentences).size();
            double sentencesProcessed = 0;
            for(List<? extends HasWord> sentence : sentences)
            {
                Tree parseTree = parser.apply(sentence);
                String parsed = treeToString(parseTree.firstChild());
                writer.write(parsed + "\n");
                System.out.print("  " + numDone);
                System.out.printf("%2.2f%% ", sentencesProcessed / totalSentences * 100.0);
                System.out.print(parsed + "\n");
                ++sentencesProcessed;
            }
            System.out.println();
            writer.close();
            ++filesDone;
        }
    }
    
    /**
     * Converts a Parse Tree into a String representation
     * @param tree - the Parse Tree to convert
     * @return the string representation
     */
    private static String treeToString(Tree tree)
    {
        if(tree.isLeaf())
            return "";
        
        String retString = "(" + tree.value();
        for(Tree node : tree.children())
            retString += treeToString(node);
        retString += ")";
        
        return retString;
    }
    
    public static void main(String args[]) throws IOException
    {
        System.out.println("Processing " + args.length + " files.");
        process(args, "80");
    }
}
